#include "nos.h"
#include "nos_model.h"

#define nos_vector_accumulate(a, out) nos_vector_add(out, a, out)

#define QUAD_SELECTED(q) (unpacker->selected_quads[q])
#define SELECT_QUAD(q) do { unpacker->selected_quads[q] = 1; } while(0)

static unsigned char visited_vertexes[MAX_VERTS];
#define VERTEX_VISITED(v) (visited_vertexes[v])

static float mins[3], maxs[3];

static void finalize(struct nos_model *model, struct nos_model_unpacker *unpacker) {
	int q, v;

	nos_memset(model->quad_normal, 0, sizeof(model->quad_normal));
	nos_memset(model->vertex_normal, 0, sizeof(model->vertex_normal));
	nos_memset(unpacker->quad_center, 0, sizeof(unpacker->quad_center));
	nos_memset(visited_vertexes, 0, sizeof(visited_vertexes));

	mins[0] = mins[1] = mins[2] =  100;
	maxs[0] = maxs[1] = maxs[2] = -100;

	for(q = 0; q < model->quads; ++q) {
		for(v = 0; v < 4; ++v) {
			float n[3];
			nos_vector_accumulate(model->vertex_pos[model->quad_index[q][v]], unpacker->quad_center[q]);
			nos_vector_cross(model->vertex_pos[model->quad_index[q][v]], model->vertex_pos[model->quad_index[q][(v+1) & 3]], n);
			nos_vector_accumulate(n, model->quad_normal[q]);
			visited_vertexes[model->quad_index[q][v]] |= unpacker->selected_quads[q];
		}
		for(v = 0; v < 4; ++v) {
			nos_vector_accumulate(model->quad_normal[q], model->vertex_normal[model->quad_index[q][v]]);
		}
		nos_vector_normalize(model->quad_normal[q]);
		nos_vector_scale(unpacker->quad_center[q], 0.25f, unpacker->quad_center[q]);
	}

	for(v = 0; v < model->vertexes; ++v) {
		if(VERTEX_VISITED(v)) {
			nos_vector_min(mins, model->vertex_pos[v], mins);
			nos_vector_max(mins, model->vertex_pos[v], maxs);
		}
		nos_vector_normalize(model->vertex_normal[v]);
	}
}

static void add_quad(struct nos_model *model, struct nos_model_unpacker *unpacker, unsigned int a, unsigned int b, unsigned int c, unsigned int d, int selected) {
	model->quad_index[model->quads][0] = a;
	model->quad_index[model->quads][1] = b;
	model->quad_index[model->quads][2] = c;
	model->quad_index[model->quads][3] = d;
	if(selected) {
		SELECT_QUAD(model->quads);
	}
	model->quads++;
}

static unsigned int add_vertex(struct nos_model *model, float *v) {
	int i;
	for(i = 0; i < model->vertexes; ++i) {
		if(model->vertex_pos[i][0] == v[0] && model->vertex_pos[i][1] == v[1] && model->vertex_pos[i][2] == v[2]) {
			return i;
		}
	}
	nos_vector_copy(v, model->vertex_pos[model->vertexes]);
	return model->vertexes++;
}

static float subdivide_displacement[MAX_VERTS][3];
static unsigned int subdivide_valence[MAX_VERTS];

static void subdivide(struct nos_model *model, struct nos_model_unpacker *unpacker, int smooth) {
	int q, v, i;
	int original_vertexes = model->vertexes, original_quads = model->quads;
	unsigned int center_id, edge_id[4]; 
	float R[3], tmp[3];
	float r_scale = smooth ? 0.5f : 1.0f;

	nos_memset(subdivide_displacement, 0, sizeof(subdivide_displacement));
	nos_memset(subdivide_valence, 0, sizeof(subdivide_valence));

	for(q = 0; q < original_quads; ++q) {
#define F unpacker->quad_center[q]
		center_id = add_vertex(model, unpacker->quad_center[q]);
		for(i = 0; i < 4; ++i) {
			v = model->quad_index[q][i];

			nos_vector_lerp(model->vertex_pos[v], model->vertex_pos[model->quad_index[q][(i+1) & 3]], 0.5f, R);
			nos_vector_scale(R, r_scale, tmp);
			edge_id[i] = add_vertex(model, tmp);

			nos_vector_mad(F, 0.25f, subdivide_displacement[edge_id[i]], subdivide_displacement[edge_id[i]]);
			nos_vector_mad(R, 2.0f, subdivide_displacement[v], subdivide_displacement[v]);
			nos_vector_add(subdivide_displacement[v], F, subdivide_displacement[v]);

			++subdivide_valence[v];
		}

		for(i = 1; i < 4; ++i) {
			add_quad(model, unpacker, model->quad_index[q][i], edge_id[i], center_id, edge_id[(i - 1) & 3], QUAD_SELECTED(q));
		}

		model->quad_index[q][1] = edge_id[0];
		model->quad_index[q][2] = center_id;
		model->quad_index[q][3] = edge_id[3];
	}
	if(smooth) {
		for(v = 0; v < model->vertexes; ++v) {
#define P model->vertex_pos[v]
			if(v < original_vertexes) {
				float iN = 1.0f / (float)subdivide_valence[v];
				nos_vector_scale(subdivide_displacement[v], iN, subdivide_displacement[v]);
				nos_vector_mad(P, (subdivide_valence[v] - 3.0f), subdivide_displacement[v], P);
				nos_vector_scale(P, iN, P);
			} else {
				nos_vector_accumulate(subdivide_displacement[v], model->vertex_pos[v]);
			}
		}
	}
}

static void new_quads(struct nos_model *model, struct nos_model_unpacker *unpacker, float lerp[MAX_QUADS][3], float lerpAmount, float add[MAX_QUADS][3]) {
	int q, i;
	for(q = 0; q < model->quads; ++q) {
		if(!QUAD_SELECTED(q)) {
			continue;
		}
		for(i = 0; i < 4; ++i) {
			if(lerp) {
				nos_vector_lerp(model->vertex_pos[model->quad_index[q][i]], lerp[q], lerpAmount, model->vertex_pos[model->vertexes+i]);
			} else {
				nos_vector_add(model->vertex_pos[model->quad_index[q][i]], add[q], model->vertex_pos[model->vertexes+i]);
			}
			add_quad(model, unpacker, model->quad_index[q][i], model->quad_index[q][(i+1) & 3], model->vertexes+((i+1) & 3), model->vertexes+i, 0);
		}
		for(i = 0; i < 4; ++i) {
			model->quad_index[q][i] = model->vertexes++;
		}
	}
}

static const unsigned char *get_value(unsigned char code, const unsigned char *bc, float *value) {
	if(code & 0x01) {
		*value = ((float)((int)(bc[0] | (bc[1]<<8)) - 32768) / 32768.0f);
		return bc + 2;
	} else {
		*value = ((float)((int)bc[0] - 127) / 127.0f);
		return bc + 1;
	}
}

static void nos_vector_Mad(float *v1, float *v2, float *v3, float *out) {
	int i;
	for(i = 0; i < 3; ++i) {
		out[i] = v1[i]*v2[i] + v3[i];
	}
}

static unsigned char new_cube[] = {
	3, 2, 1, 0,
	0, 1, 5, 4,
	1, 2, 6, 5,
	2, 3, 7, 6,
	3, 0, 4, 7,
	4, 5, 6, 7
};

const unsigned char *nos_model_command(const unsigned char *bc, struct nos_model_unpacker *unpacker, struct nos_model *model) {
	unsigned char code;
	int v, q, i;
	float value[3];
	unsigned char xyz[3] = {0x8, 0x4, 0x2};

	finalize(model, unpacker);
	code = *bc++;
	if((code & 0xf0) == 0x00) {
		switch(code & 0x0f) {
		case 0:
			// end build
			return NULL;
		case 1:
		case 2:
			// smooth - catmul clark
			// subdivide
			subdivide(model, unpacker, (code & 0x0f) == 0x01);
			return bc;
		case 3:
			// add cube
			for(i = 0; i < 24; i += 4) {
				add_quad(model, unpacker, model->vertexes+new_cube[i+0], model->vertexes+new_cube[i+1], model->vertexes+new_cube[i+2], model->vertexes+new_cube[i+3], 1);
			}

			{ float v[3] = {-1.0f,  1.0f, -1.0f}; add_vertex(model, v); }
			{ float v[3] = { 1.0f,  1.0f, -1.0f}; add_vertex(model, v); }
			{ float v[3] = { 1.0f,  1.0f,  1.0f}; add_vertex(model, v); }
			{ float v[3] = {-1.0f,  1.0f,  1.0f}; add_vertex(model, v); }

			{ float v[3] = {-1.0f, -1.0f, -1.0f}; add_vertex(model, v); }
			{ float v[3] = { 1.0f, -1.0f, -1.0f}; add_vertex(model, v); }
			{ float v[3] = { 1.0f, -1.0f,  1.0f}; add_vertex(model, v); }
			{ float v[3] = {-1.0f, -1.0f,  1.0f}; add_vertex(model, v); }
			
			break;
		case 4:
			// inset
			new_quads(model, unpacker, unpacker->quad_center, 0.5f, NULL);
			break;
		case 5:
			// extrude
			new_quads(model, unpacker, NULL, 0.0f, model->quad_normal);
			break;
		case 6:
			// taper
			new_quads(model, unpacker, model->quad_normal, 0.5f, NULL);
			break;
		case 8:
			// cage
			new_quads(model, unpacker, unpacker->quad_center, 0.9f, NULL);
			/* fallthrough */
		case 7:
			// delete
			for(q = 0, i = 0; q < model->quads; ++i) {
				if(!QUAD_SELECTED(i)) {
					++q;
					continue;
				}
				--model->quads;
				nos_memcpy(&model->quad_index[q][0], &model->quad_index[q+1][0], sizeof(model->quad_index[q])*(model->quads - q));
			}
			nos_memset(unpacker->selected_quads, 0, sizeof(unpacker->selected_quads));
			break;
		}
	} else if((code & 0xf0) == 0xf0) { // selection by index
		i = (code & 0x0f) + 1;
		while(i--) {
			q = (bc[0] | (bc[1]<<8));
			unpacker->selected_quads[q] ^= 1;
			bc += 2;
		}
	} else if((code & 0xf0) == 0xe0) { // selection by procedual
		switch(code & 0x0f) {
		case 0: // select all
			nos_memset(unpacker->selected_quads, 0xff, sizeof(unpacker->selected_quads));
			break;
		case 1: // deselect all
			nos_memset(unpacker->selected_quads, 0, sizeof(unpacker->selected_quads));
			break;
		case 2: // invert
			for(q = 0; q < model->quads; ++q) {
				unpacker->selected_quads[q] ^= 1;
			}
			break;
		case 3: // grow selection
			for(q = 0; q < model->quads; ++q) {
				for(i = 0; i < 4; ++i) {
					unpacker->selected_quads[q] |= VERTEX_VISITED(model->quad_index[q][i]);
				}
			}
			break;
		case 4: // select adjacent
			for(q = 0; q < model->quads; ++q) {
				int s = 0;
				for(i = 0; i < 4; ++i) {
					s |= VERTEX_VISITED(model->quad_index[q][i]);
				}
				unpacker->selected_quads[q] ^= s;
			}
			break;
		case 5: // x select
		case 6: // y select
		case 7: // z select
			i = (code & 0x03) - 1;
			for(q = 0; q < model->quads; ++q) {
				for(v = 0; v < 4; ++v) {
					unpacker->selected_quads[q] |= model->vertex_pos[model->quad_index[q][v]][i] - mins[i] < (maxs[i] - mins[i]);
				}
			}
			break;
		}
	} else {
		if((code & 0xe0) == 0x80) {
			value[0] = value[1] = value[2] = 0.25f;
		} else {
			value[0] = value[1] = value[2] = 0.0f;
		}
		if((code & 0xee) == 0x80 || (code & 0xee) == 0xa0) { // scale gets uniform scale, translation gets movement distance, rotate gets nothing
			bc = get_value(code, bc, &value[2]);
			value[0] = value[1] = value[2];
		} else {
			for(i = 0; i < 3; ++i) {
				if(code & xyz[i]) {
					bc = get_value(code, bc, &value[i]);
				}
			}
		}
		nos_vector_scale(value, 4.0f, value);

		// apply to all vertexes
		if((code & 0x10) == 0x10) {
			nos_memset(visited_vertexes, 0xff, sizeof(visited_vertexes));
		}

		if((code & 0xe0) == 0x80) { // scale
			for(v = 0; v < model->vertexes; ++v) {
				if(!VERTEX_VISITED(v)) {
					continue;
				}
				nos_vector_mul(model->vertex_pos[v], value, model->vertex_pos[v]);
			}
		} else if((code & 0xe0) == 0xa0) {
			if((code & 0x0e) == 0x00) { // translate by normals
				for(q = 0; q < model->quads; ++q) {
					if(!QUAD_SELECTED(q)) {
						continue;
					}
					for(i = 0; i < 4; ++i) {
						nos_vector_Mad(model->quad_normal[q], value, model->vertex_pos[model->quad_index[q][i]], model->vertex_pos[model->quad_index[q][i]]);
					}
				}
			} else { // translate by value
				for(v = 0; v < model->vertexes; ++v) {
					if(!VERTEX_VISITED(v)) {
						continue;
					}
					nos_vector_add(model->vertex_pos[v], value, model->vertex_pos[v]);
				}
			}
		} else { // rotate
			float matrix[16];
			float pos[3];

			nos_matrix_identity(matrix);
			nos_matrix_rotate(matrix, value[0]*90.0f, 1.0f, 0.0f, 0.0f);
			nos_matrix_rotate(matrix, value[1]*90.0f, 0.0f, 1.0f, 0.0f);
			nos_matrix_rotate(matrix, value[2]*90.0f, 0.0f, 0.0f, 1.0f);

			for(v = 0; v < model->vertexes; ++v) {
				if(!VERTEX_VISITED(v)) {
					continue;
				}
				nos_vector_copy(model->vertex_pos[v], pos);
				model->vertex_pos[v][0] = matrix[ 0]*pos[0] + matrix[ 4]*pos[1] + matrix[ 8]*pos[2];
				model->vertex_pos[v][1] = matrix[ 1]*pos[0] + matrix[ 5]*pos[1] + matrix[ 9]*pos[2];
				model->vertex_pos[v][2] = matrix[ 2]*pos[0] + matrix[ 6]*pos[1] + matrix[10]*pos[2];
			}
		}
	}
	return bc;
}

void nos_model_unpack(const unsigned char *bc, struct nos_model_unpacker *unpacker, struct nos_model *model) {
	nos_memset(unpacker->selected_quads, 0, sizeof(unpacker->selected_quads));
	model->quads = 0;
	model->vertexes = 0;
	nos_model_command((const unsigned char *)"\x03", unpacker, model);
	while((bc = nos_model_command(bc, unpacker, model)) != NULL) ;
}
