#ifndef CORE_MATRIX_H
#define CORE_MATRIX_H

#include "scalar.h"

// 4x4 matrix

void matrix_identity(vec m[16]);
void matrix_copy(const vec m[16], vec out[16]);
void matrix_copy_transpose(const vec m[16], vec out[16]);
void matrix_mult(const vec m1[16], const vec m2[16], vec out[16]);
void matrix_rotate(vec m[16], vec angle, vec x, vec y, vec z);
void matrix_translate(vec m[16], vec x, vec y, vec z);
void matrix_scale(vec m[16], vec x, vec y, vec z);
void matrix_perspective(vec m[16], vec fovy, vec aspect, vec zNear, vec zFar);
void matrix_frustum(vec m[16], vec left, vec right, vec bottom, vec top, vec nearVal, vec farVal);
void matrix_ortho(vec m[16], vec left, vec right, vec bottom, vec top, vec nearVal, vec farVal);
void matrix_invert(vec m[16]);

#endif