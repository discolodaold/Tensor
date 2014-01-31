#include "matrix.h"

#include "vector.h"

#include <string.h>

#define radians(X) (X * 3.14159265358979323846f / 180.0f)

void matrix_identity(vec m[16]) {
	m[0] = m[5] = m[10] = m[15] = 1.0f;
	m[1] = m[2] = m[3] = m[4] = m[6] = m[7] = m[8] = m[9] = m[11] = m[12] = m[13] = m[14] = 0.0f;
}

void matrix_copy(const vec m[16], vec out[16]) {
	memcpy(out, m, sizeof(vec) * 16);
}

void matrix_copy_transpose(const vec m[16], vec out[16]) {
	out[ 0] = m[ 0];
	out[ 1] = m[ 4];
	out[ 2] = m[ 8];
	out[ 3] = m[12];
	out[ 4] = m[ 1];
	out[ 5] = m[ 5];
	out[ 6] = m[ 9];
	out[ 7] = m[13];
	out[ 8] = m[ 2];
	out[ 9] = m[ 6];
	out[10] = m[10];
	out[11] = m[14];
	out[12] = m[ 3];
	out[13] = m[ 7];
	out[14] = m[11];
	out[15] = m[15];
}

void matrix_mult(const vec m1[16], const vec m2[16], vec out[16]) {
	out[ 0] = m1[ 0]*m2[ 0] + m1[ 4]*m2[ 1] + m1[ 8]*m2[ 2] + m1[12]*m2[ 3];
	out[ 1] = m1[ 1]*m2[ 0] + m1[ 5]*m2[ 1] + m1[ 9]*m2[ 2] + m1[13]*m2[ 3];
	out[ 2] = m1[ 2]*m2[ 0] + m1[ 6]*m2[ 1] + m1[10]*m2[ 2] + m1[14]*m2[ 3];
	out[ 3] = m1[ 3]*m2[ 0] + m1[ 7]*m2[ 1] + m1[11]*m2[ 2] + m1[15]*m2[ 3];
	out[ 4] = m1[ 0]*m2[ 4] + m1[ 4]*m2[ 5] + m1[ 8]*m2[ 6] + m1[12]*m2[ 7];
	out[ 5] = m1[ 1]*m2[ 4] + m1[ 5]*m2[ 5] + m1[ 9]*m2[ 6] + m1[13]*m2[ 7];
	out[ 6] = m1[ 2]*m2[ 4] + m1[ 6]*m2[ 5] + m1[10]*m2[ 6] + m1[14]*m2[ 7];
	out[ 7] = m1[ 3]*m2[ 4] + m1[ 7]*m2[ 5] + m1[11]*m2[ 6] + m1[15]*m2[ 7];
	out[ 8] = m1[ 0]*m2[ 8] + m1[ 4]*m2[ 9] + m1[ 8]*m2[10] + m1[12]*m2[11];
	out[ 9] = m1[ 1]*m2[ 8] + m1[ 5]*m2[ 9] + m1[ 9]*m2[10] + m1[13]*m2[11];
	out[10] = m1[ 2]*m2[ 8] + m1[ 6]*m2[ 9] + m1[10]*m2[10] + m1[14]*m2[11];
	out[11] = m1[ 3]*m2[ 8] + m1[ 7]*m2[ 9] + m1[11]*m2[10] + m1[15]*m2[11];
	out[12] = m1[ 0]*m2[12] + m1[ 4]*m2[13] + m1[ 8]*m2[14] + m1[12]*m2[15];
	out[13] = m1[ 1]*m2[12] + m1[ 5]*m2[13] + m1[ 9]*m2[14] + m1[13]*m2[15];
	out[14] = m1[ 2]*m2[12] + m1[ 6]*m2[13] + m1[10]*m2[14] + m1[14]*m2[15];
	out[15] = m1[ 3]*m2[12] + m1[ 7]*m2[13] + m1[11]*m2[14] + m1[15]*m2[15];
}

void matrix_rotate(vec m[16], vec angle, vec x, vec y, vec z) {
	vec v[3] = {x, y, z}, axis[3], temp[3];
	vec c, s;
	vec m2[16], m1[16];

	angle = radians(angle);
	vec_sincos(angle, s, c);

	vec3_normalize_into(v, axis);
	temp[0] = axis[0]*(1 - c);
	temp[1] = axis[1]*(1 - c);
	temp[2] = axis[2]*(1 - c);

	m2[ 0] = temp[0]*axis[0] + c;
	m2[ 1] = temp[0]*axis[1] + s * axis[2];
	m2[ 2] = temp[0]*axis[2] - s * axis[1];

	m2[ 4] = temp[1]*axis[0] - s * axis[2];
	m2[ 5] = temp[1]*axis[1] + c;
	m2[ 6] = temp[1]*axis[2] + s * axis[0];

	m2[ 8] = temp[2]*axis[0] + s * axis[1];
	m2[ 9] = temp[2]*axis[1] - s * axis[0];
	m2[10] = temp[2]*axis[2] + c;

	memcpy(m1, m, sizeof(m1));

	m[ 0] = m1[ 0]*m2[ 0] + m1[ 4]*m2[ 1] + m1[ 8]*m2[ 2];
	m[ 1] = m1[ 1]*m2[ 0] + m1[ 5]*m2[ 1] + m1[ 9]*m2[ 2];
	m[ 2] = m1[ 2]*m2[ 0] + m1[ 6]*m2[ 1] + m1[10]*m2[ 2];
	m[ 3] = m1[ 3]*m2[ 0] + m1[ 7]*m2[ 1] + m1[11]*m2[ 2];
	m[ 4] = m1[ 0]*m2[ 4] + m1[ 4]*m2[ 5] + m1[ 8]*m2[ 6];
	m[ 5] = m1[ 1]*m2[ 4] + m1[ 5]*m2[ 5] + m1[ 9]*m2[ 6];
	m[ 6] = m1[ 2]*m2[ 4] + m1[ 6]*m2[ 5] + m1[10]*m2[ 6];
	m[ 7] = m1[ 3]*m2[ 4] + m1[ 7]*m2[ 5] + m1[11]*m2[ 6];
	m[ 8] = m1[ 0]*m2[ 8] + m1[ 4]*m2[ 9] + m1[ 8]*m2[10];
	m[ 9] = m1[ 1]*m2[ 8] + m1[ 5]*m2[ 9] + m1[ 9]*m2[10];
	m[10] = m1[ 2]*m2[ 8] + m1[ 6]*m2[ 9] + m1[10]*m2[10];
	m[11] = m1[ 3]*m2[ 8] + m1[ 7]*m2[ 9] + m1[11]*m2[10];
}

void matrix_translate(vec m[16], vec x, vec y, vec z) {
	m[12] += m[ 0]*x + m[ 4]*y + m[ 8]*z;
	m[13] += m[ 1]*x + m[ 5]*y + m[ 9]*z;
	m[14] += m[ 2]*x + m[ 6]*y + m[10]*z;
	m[15] += m[ 3]*x + m[ 7]*y + m[11]*z;
}

void matrix_scale(vec m[16], vec x, vec y, vec z) {
	m[ 0] *= x;
	m[ 1] *= x;
	m[ 2] *= x;
	m[ 3] *= x;
	m[ 4] *= y;
	m[ 5] *= y;
	m[ 6] *= y;
	m[ 7] *= y;
	m[ 8] *= z;
	m[ 9] *= z;
	m[10] *= z;
	m[11] *= z;
}

void matrix_perspective(float m[16], float fovy, float aspect, float zNear, float zFar) {
	vec range = radians(fovy / 2.0f), left, right, bottom, top;

	vec_tan(range, range);
	range *= zNear;
	left = -range * aspect;
	right = range * aspect;
	bottom = -range;
	top = range;

	{	vec m2[16] = {
			(2.0f * zNear) / (right - left), 0.0f, 0.0f, 0.0f,
			0.0f, (2.0f * zNear) / (top - bottom), 0.0f, 0.0f,
			0.0f, 0.0f, -(zFar + zNear) / (zFar - zNear), -1.0f,
			0.0f, 0.0f, -(2.0f * zFar * zNear) / (zFar - zNear), 0.0f
		}, m1[16];

		memcpy(m1, m, sizeof(m1));

		m[ 0] *= m2[ 0];
		m[ 1] *= m2[ 0];
		m[ 2] *= m2[ 0];
		m[ 3] *= m2[ 0];
		m[ 4] *= m2[ 5];
		m[ 5] *= m2[ 5];
		m[ 6] *= m2[ 5];
		m[ 7] *= m2[ 5];
		m[ 8] = m1[ 8]*m2[10] - m[12];
		m[ 9] = m1[ 9]*m2[10] - m[13];
		m[10] = m1[10]*m2[10] - m[14];
		m[11] = m1[11]*m2[10] - m[15];
		m[12] = m1[ 8]*m2[14];
		m[13] = m1[ 9]*m2[14];
		m[14] = m1[10]*m2[14];
		m[15] = m1[11]*m2[14];
	}
}

void matrix_frustum(vec m[16], vec left, vec right, vec bottom, vec top, vec nearVal, vec farVal) {
	vec m2[16], m1[16];

	m2[ 0] = (2 * nearVal) / (right - left);
	m2[ 5] = (2 * nearVal) / (top - bottom);
	m2[ 8] = (right + left) / (right - left);
	m2[ 9] = (top + bottom) / (top - bottom);
	m2[10] = -((farVal + nearVal) / (farVal - nearVal));
	m2[11] = -1.0f;
	m2[14] = -((2 * farVal * nearVal) / (farVal - nearVal));

	memcpy(m1, m, sizeof(m1));

	m[ 0] = m1[ 0]*m2[ 0];
	m[ 1] = m1[ 1]*m2[ 0];
	m[ 2] = m1[ 2]*m2[ 0];
	m[ 3] = m1[ 3]*m2[ 0];
	m[ 4] = m1[ 4]*m2[ 5];
	m[ 5] = m1[ 5]*m2[ 5];
	m[ 6] = m1[ 6]*m2[ 5];
	m[ 7] = m1[ 7]*m2[ 5];
	m[ 8] = m1[ 0]*m2[ 8] + m1[ 4]*m2[ 9] + m1[ 8]*m2[10] + m1[12]*m2[11];
	m[ 9] = m1[ 1]*m2[ 8] + m1[ 5]*m2[ 9] + m1[ 9]*m2[10] + m1[13]*m2[11];
	m[10] = m1[ 2]*m2[ 8] + m1[ 6]*m2[ 9] + m1[10]*m2[10] + m1[14]*m2[11];
	m[11] = m1[ 3]*m2[ 8] + m1[ 7]*m2[ 9] + m1[11]*m2[10] + m1[15]*m2[11];
	m[12] = m1[ 8]*m2[14];
	m[13] = m1[ 9]*m2[14];
	m[14] = m1[10]*m2[14];
	m[15] = m1[11]*m2[14];
}

void matrix_ortho(vec m[16], vec left, vec right, vec bottom, vec top, vec nearVal, vec farVal) {
	vec m2[16], m1[16];

	m2[ 0] = 2 / (right - left);
	m2[ 5] = 2 / (top - bottom);
	m2[10] = -2 / (farVal - nearVal);
	m2[12] = -((right + left) / (right - left));
	m2[13] = -((top + bottom) / (top - bottom));
	m2[14] = -((farVal + nearVal) / (farVal - nearVal));

	memcpy(m1, m, sizeof(m1));

	m[ 0] = m1[ 0]*m2[ 0];
	m[ 1] = m1[ 1]*m2[ 0];
	m[ 2] = m1[ 2]*m2[ 0];
	m[ 3] = m1[ 3]*m2[ 0];
	m[ 4] = m1[ 4]*m2[ 5];
	m[ 5] = m1[ 5]*m2[ 5];
	m[ 6] = m1[ 6]*m2[ 5];
	m[ 7] = m1[ 7]*m2[ 5];
	m[ 8] = m1[ 8]*m2[10];
	m[ 9] = m1[ 9]*m2[10];
	m[10] = m1[10]*m2[10];
	m[11] = m1[11]*m2[10];
	m[12] = m1[ 0]*m2[12] + m1[ 4]*m2[13] + m1[ 8]*m2[14] + m1[12];
	m[13] = m1[ 1]*m2[12] + m1[ 5]*m2[13] + m1[ 9]*m2[14] + m1[13];
	m[14] = m1[ 2]*m2[12] + m1[ 6]*m2[13] + m1[10]*m2[14] + m1[14];
	m[15] = m1[ 3]*m2[12] + m1[ 7]*m2[13] + m1[11]*m2[14] + m1[15];
}

/** ================================================================================================================
 * taken from darkplaces/matrixlib.c 05/07/2010 Adapted from code contributed to Mesa by David Moore (Mesa 7.6
 * under SGI Free License B - which is MIT/X11-type) added helper for common subexpression elimination by eihrul,
 * and other optimizations by div0
 =================================================================================================================== */
void matrix_invert(vec m[16]) {
	/* note: orientation does not matter, as transpose(invert(transpose(m))) == invert(m),
	   proof: transpose(invert(transpose(m))) * m
	   transpose(invert(transpose(m))) * transpose(transpose(m))
	   transpose(transpose(m) * invert(transpose(m))) = transpose(identity) = identity
	*/

	// this seems to help gcc's common subexpression elimination, and also makes the code look nicer
	vec m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33, det;

	m00 = m[ 0];
	m01 = m[ 1];
	m02 = m[ 2];
	m03 = m[ 3];
	m10 = m[ 4];
	m11 = m[ 5];
	m12 = m[ 6];
	m13 = m[ 7];
	m20 = m[ 8];
	m21 = m[ 9];
	m22 = m[10];
	m23 = m[11];
	m30 = m[12];
	m31 = m[13];
	m32 = m[14];
	m33 = m[15];

	// calculate the adjoint
	m[ 0] =  (m11*(m22*m33 - m23*m32) - m21*(m12*m33 - m13*m32) + m31*(m12*m23 - m13*m22));
	m[ 1] = -(m01*(m22*m33 - m23*m32) - m21*(m02*m33 - m03*m32) + m31*(m02*m23 - m03*m22));
	m[ 2] =  (m01*(m12*m33 - m13*m32) - m11*(m02*m33 - m03*m32) + m31*(m02*m13 - m03*m12));
	m[ 3] = -(m01*(m12*m23 - m13*m22) - m11*(m02*m23 - m03*m22) + m21*(m02*m13 - m03*m12));
	m[ 4] = -(m10*(m22*m33 - m23*m32) - m20*(m12*m33 - m13*m32) + m30*(m12*m23 - m13*m22));
	m[ 5] =  (m00*(m22*m33 - m23*m32) - m20*(m02*m33 - m03*m32) + m30*(m02*m23 - m03*m22));
	m[ 6] = -(m00*(m12*m33 - m13*m32) - m10*(m02*m33 - m03*m32) + m30*(m02*m13 - m03*m12));
	m[ 7] =  (m00*(m12*m23 - m13*m22) - m10*(m02*m23 - m03*m22) + m20*(m02*m13 - m03*m12));
	m[ 8] =  (m10*(m21*m33 - m23*m31) - m20*(m11*m33 - m13*m31) + m30*(m11*m23 - m13*m21));
	m[ 9] = -(m00*(m21*m33 - m23*m31) - m20*(m01*m33 - m03*m31) + m30*(m01*m23 - m03*m21));
	m[10] =  (m00*(m11*m33 - m13*m31) - m10*(m01*m33 - m03*m31) + m30*(m01*m13 - m03*m11));
	m[11] = -(m00*(m11*m23 - m13*m21) - m10*(m01*m23 - m03*m21) + m20*(m01*m13 - m03*m11));
	m[12] = -(m10*(m21*m32 - m22*m31) - m20*(m11*m32 - m12*m31) + m30*(m11*m22 - m12*m21));
	m[13] =  (m00*(m21*m32 - m22*m31) - m20*(m01*m32 - m02*m31) + m30*(m01*m22 - m02*m21));
	m[14] = -(m00*(m11*m32 - m12*m31) - m10*(m01*m32 - m02*m31) + m30*(m01*m12 - m02*m11));
	m[15] =  (m00*(m11*m22 - m12*m21) - m10*(m01*m22 - m02*m21) + m20*(m01*m12 - m02*m11));

	/// calculate the determinant (as inverse == 1/det * adjoint, adjoint * m == identity * det, so this calculates the det)
	det = m00*m[ 0] + m10*m[ 1] + m20*m[ 2] + m30*m[ 3];
	if(det == 0.0f)
		return;

	// multiplications are faster than divisions, usually
	det = 1.0f / det;

	// manually unrolled loop to multiply all matrix elements by 1/det
	m[ 0] *= det;
	m[ 1] *= det;
	m[ 2] *= det;
	m[ 3] *= det;
	m[ 4] *= det;
	m[ 5] *= det;
	m[ 6] *= det;
	m[ 7] *= det;
	m[ 8] *= det;
	m[ 9] *= det;
	m[10] *= det;
	m[11] *= det;
	m[12] *= det;
	m[12] *= det;
	m[13] *= det;
	m[14] *= det;
}
