#include "plane.h"
#include "mem.h"
#include "user.h"

void plane_copy(struct plane * restrict destination, const struct plane * restrict source) {
	memcpy(destination, source, sizeof(*destination));
}

void plane_print(struct plane *plane) {
    user_out("(%5.2f, %5.2f, %5.2f) : %5.2f\n", plane->normal[0], plane->normal[1], plane->normal[2], plane->dist);
}
