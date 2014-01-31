#ifndef CORE_VECTOR_H
#define CORE_VECTOR_H

#include "scalar.h"

typedef vec vec2[2];
typedef vec vec3[3];
typedef vec vec4[4];

extern vec3 vec3_origin;

#define vec3_dot_product(x,y) (x[0]*y[0] + x[1]*y[1] + x[2]*y[2])
#define vec3_subtract(a,b,c) (c[0] = a[0] - b[0], c[1] = a[1] - b[1], c[2] = a[2] - b[2])
#define vec3_add(a,b,c) (c[0] = a[0] + b[0], c[1] = a[1] + b[1], c[2] = a[2] + b[2])
#define vec3_copy(a,b) (b[0] = a[0], b[1] = a[1], b[2] = a[2])
#define vec3_ma(va, scale, vb, vc) (vc[0] = va[0] + scale*vb[0], vc[1] = va[1] + scale*vb[1], vc[2] = va[2] + scale*vb[2])
#define vec3_scale(a, s, b) (b[0] = a[0]*s, b[1] = a[1]*s, b[2] = a[2]*s)
#define vec3_cross_product(v1, v2, cross) (cross[0] = v1[1]*v2[2] - v1[2]*v2[1], cross[1] = v1[2]*v2[0] - v1[0]*v2[2], cross[2] = v1[0]*v2[1] - v1[1]*v2[0])

vec vec3_normalize_into(const vec3, vec3);
#define vec3_normalize(v) vec3_normalize_into(v, v)
void vec3_print(vec3);

#endif
