#ifndef CORE_MAP_H
#define CORE_MAP_H

#include "str2ptr.h"
#include "plane.h"
#include "array.h"
#include "vector.h"

struct map_face {
	vec3         planepts[3];
	struct plane plane;

	struct {
		const char *name;
		float      shift[2];
		float      rotate;
		float      scale[2];
	} texture;

	int contents;
	int flags;
	int value;

	struct map_brush *brush;
};

struct map_brush {
	array(struct map_face) faces;
	struct map_entity      *entity;
};

struct map_entity {
	array(struct map_brush) brushes;
	struct str2ptr          *attributes;
	struct map              *map;
};

struct map {
	array(struct map_entity) entities;
	struct map_entity        *world;
};

struct map *load_map(unsigned char *);

void map_entity_set(struct map_entity *, const char *, const char *);

#endif
