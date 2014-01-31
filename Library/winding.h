#ifndef CORE_WINDING_H
#define CORE_WINDING_H

#include "plane.h"
#include "array.h"

struct winding {
	struct plane plane;
	array(vec3) points;
};

void winding_init(struct winding *winding);
void winding_from_plane(struct winding * restrict winding, const struct plane * restrict plane);
void winding_copy(struct winding * restrict destination, const struct winding * restrict source);
void winding_clip(struct winding * restrict winding, const struct plane * restrict plane, int keep_on);
void winding_print(const struct winding *winding);
void winding_free(struct winding *winding);

#endif
