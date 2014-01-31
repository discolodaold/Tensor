#ifndef CORE_PLANE_H
#define CORE_PLANE_H

#include "vector.h"

#define	SIDE_FRONT 0
#define	SIDE_ON    2
#define	SIDE_BACK  1

#define PLANE_EPSILON 0.001

struct plane {
    vec3 normal;
    vec  dist;
};

void plane_init(struct plane *plane);
void plane_copy(struct plane *destination, const struct plane *source);
void plane_print(struct plane *plane);

#endif
