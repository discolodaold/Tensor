#define MAX_QUADS 65536
#define MAX_VERTS 65536

struct nos_model {
	unsigned short quads;
	unsigned short quad_index[MAX_QUADS][4];
	float          quad_normal[MAX_QUADS][3];

	unsigned short vertexes;
	float          vertex_pos[MAX_VERTS][3];
	float          vertex_normal[MAX_VERTS][3];
};

struct nos_model_unpacker {
	unsigned char selected_quads[MAX_QUADS];
	unsigned char deleted_quads[MAX_QUADS];
	float         quad_center[MAX_QUADS][3];
};

// 0000 xxxx extra
// 100x xxxx scale
// 101x xxxx translate
// 110x xxxx rotate
// 1110 xxxx selection
// 1111 nnnn invert selection on n+1 quads (up to 17)

// 0000 xxxx extra 1
//      0000 end build
//      0001 smooth
//      0010 subdivide
//      0011 add cube
//      0100 inset
//      0101 extrude
//      0110 taper
//      0111 delete
//      1000 cage
//      1001 harden outside edges
//      1010 harden edges
//      1011 harden shared edges

// 100x xxxx scale
//    1      for full object
//    0      for selected quads
//      000  uniform
//      1    x axis
//       1   y axis
//        1  z axis
//         0 quarter float
//         1 half float

// 101x xxxx translate
//    1      for full object
//    0      for selected quads
//      000  quad normal
//      1    x axis
//       1   y axis
//        1  z axis
//         0 quarter float
//         1 half float

// 110x xxxx rotate
//    ?      unused
//      000  unused
//      1    rotate by x
//       1   rotate by y
//        1  rotate by z
//         0 quarter float
//         1 half float

// 1110 xxxx selection
//      0000 select all
//      0001 deselect all
//      0010 invert selection
//      0011 grow selection
//      0100 select adjacent
//      0101 x select
//      0110 y select
//      0111 z select

// 1111 nnnn invert selection on n+1 quads (up to 17)

const unsigned char *nos_model_command(const unsigned char *, struct nos_model_unpacker *, struct nos_model *);
void nos_model_unpack(const unsigned char *, struct nos_model_unpacker *, struct nos_model *);
