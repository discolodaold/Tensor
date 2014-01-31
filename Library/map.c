#include "map.h"
#include "tokenizer.h"
#include "user.h"
#include "str.h"

#include <stdlib.h>

unsigned int _parse_brush(struct map_entity *ent, struct tokenizer *tokenizer) {
	struct map_brush b;
	struct map_face f;
	struct token t;
	int i, j;

	array_init(b.faces);
	b.entity = ent;

	do {
		if(!tokenizer_next(tokenizer, &t, 1)) {
			break;
		}

		if(token_is(&t, "}")) {
			break;
		}

		// read the three point plane definition
		for(i = 0; i < 3; i++) {
			if(i != 0)
				tokenizer_next(tokenizer, &t, 1);
			if(!token_is(&t, "(")) {
				user_err("parsing brush");
			}

			for(j = 0; j < 3; j++) {
				tokenizer_next(tokenizer, &t, 0);
				f.planepts[i][j] = (vec)atof(t.ptr);
			}

			tokenizer_next(tokenizer, &t, 0);
			if(!token_is(&t, ")")) {
				user_err("parsing brush");
			}
		}

		// read the texturedef
		tokenizer_next(tokenizer, &t, 0);
		f.texture.name = token_copy(&t);
		tokenizer_next(tokenizer, &t, 0);
		f.texture.shift[0] = (vec)atof(t.ptr);
		tokenizer_next(tokenizer, &t, 0);
		f.texture.shift[1] = (vec)atof(t.ptr);
		tokenizer_next(tokenizer, &t, 0);
		f.texture.rotate = (vec)atof(t.ptr);
		tokenizer_next(tokenizer, &t, 0);
		f.texture.scale[0] = (vec)atof(t.ptr);
		tokenizer_next(tokenizer, &t, 0);
		f.texture.scale[1] = (vec)atof(t.ptr);

		if(tokenizer_available(tokenizer)) {
			tokenizer_next(tokenizer, &t, 0);
			f.contents = atoi(t.ptr);
			tokenizer_next(tokenizer, &t, 0);
			f.flags = atoi(t.ptr);
			tokenizer_next(tokenizer, &t, 0);
			f.value = atoi(t.ptr);
		}

		array_append(b.faces, f);
	} while(1);

	array_append(ent->brushes, b);

	return 1;
}

unsigned int _parse_entity(struct map *map, struct tokenizer *tokenizer) {
	struct token t;
	struct map_entity ent;

	if(!tokenizer_next(tokenizer, &t, 1)) {
		return 0;
	}

	if(!token_is(&t, "{")) {
		user_err("parse entity: { not found");
		return 0;
	}

	array_init(ent.brushes);
	ent.attributes = new_str2ptr();
	ent.map = map;

	do {
		if(!tokenizer_next(tokenizer, &t, 1)) {
			user_err("ParseEntity: EOF without closing brace");
		}
		if(token_is(&t, "}")) {
			break;
		}
		if(token_is(&t, "{")) {
			_parse_brush(&ent, tokenizer);
		} else {
			const char *key = token_copy(&t);
			const char *value;
			tokenizer_next(tokenizer, &t, 0);
			value = token_copy(&t);
			map_entity_set(&ent, key, value);
			free((void *)key);
			free((void *)value);
		}
	} while(1);

	array_append(map->entities, ent);

	return 1;
}

struct map *load_map(unsigned char *data) {
	struct map *result = malloc(sizeof(*result));
	struct tokenizer *tokenizer = new_tokenizer(data);

	array_init(result->entities);
	result->world = NULL;

	while(_parse_entity(result, tokenizer)) ;

	if(!result->world) {
		free(result);
		user_err("No worldspawn in map.\n");
		return NULL;
	}

	tokenizer_free(tokenizer);

	return result;
}

void map_entity_set(struct map_entity *ent, const char *key, const char *value) {
	char *old = (char *)str2ptr_set(ent->attributes, key, (void *)string_dup(value));
	if(old) {
		free(old);
	}
}
