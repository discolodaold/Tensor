#include "vector.h"

#include "user.h"

#include <math.h>

vec3 vec3_origin = {0.0, 0.0, 0.0};

vec vec3_normalize_into(const vec3 in, vec3 out) {
	vec length, ilength;
	
	length = (vec)sqrt(vec3_dot_product(in, in));

	if(length == 0) {
		return (vec)0.0;
	}

	ilength = (vec)1.0 / length;
	vec3_scale(in, ilength, out);

	return length;
}

void vec3_print(vec3 vector) {
	user_out("(%5.2f, %5.2f, %5.2f)\n", vector[0], vector[1], vector[2]);
}
