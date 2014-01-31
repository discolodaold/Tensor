#include "winding.h"

#include "mem.h"
#include "user.h"

#include <math.h>

void winding_init(struct winding *winding) {
	plane_init(&winding->plane);
	array_init(winding->points);
}

void winding_from_plane(struct winding * restrict winding, const struct plane * restrict plane) {
	int i, x;
	vec max, v;
	vec3 org, vright, vup;

	// find the major axis
	max = -BOGUS_RANGE;
	x = -1;
	for(i = 0; i < 3; i++) {
		v = (vec)fabs(plane->normal[i]);
		if(v > max) {
			x = i;
			max = v;
		}
	}
	if(x == -1)
		user_err("winding_from_plane: no axis found");

	vec3_copy(vec3_origin, vup);
	switch(x) {
	case 0:
	case 1:
		vup[2] = 1;
		break;
	case 2:
		vup[0] = 1;
		break;
	}

	v = vec3_dot_product(vup, plane->normal);
	vec3_ma(vup, -v, plane->normal, vup);
	vec3_normalize(vup);

	vec3_scale(plane->normal, plane->dist, org);

	vec3_cross_product(vup, plane->normal, vright);

	vec3_scale(vup, 8192, vup);
	vec3_scale(vright, 8192, vright);

	// project a really big	axis aligned box onto the plane
	plane_copy(&winding->plane, plane);
	array_resize(winding->points, 4);

	vec3_subtract(org, vright, winding->points.item[0]);
	vec3_add(winding->points.item[0], vup, winding->points.item[0]);

	vec3_add(org, vright, winding->points.item[1]);
	vec3_add(winding->points.item[1], vup, winding->points.item[1]);

	vec3_add(org, vright, winding->points.item[2]);
	vec3_subtract(winding->points.item[2], vup, winding->points.item[2]);

	vec3_subtract(org, vright, winding->points.item[3]);
	vec3_subtract(winding->points.item[3], vup, winding->points.item[3]);
}

void winding_copy(struct winding * restrict destination, const struct winding * restrict source) {
	plane_copy(&destination->plane, &source->plane);
	array_free(destination->points);
	array_resize(destination->points, source->points.items);
	memcpy(destination->points.item, source->points.item, sizeof(*destination->points.item) * destination->points.items);
}

void winding_clip(struct winding * restrict winding, const struct plane * restrict plane, int keep_on) {
	vec dists[winding->points.items], *p1, *p2;
	vec dot;
	int sides[winding->points.items];
	int counts[3];
	unsigned int i, j, num_points;
	vec3 mid;
	array(vec3) points;

	counts[0] = counts[1] = counts[2] = 0;

	// determine sides for each point
	for(i = 0; i < winding->points.items; i++) {
		dists[i] = vec3_dot_product(winding->points.item[i], plane->normal) - plane->dist;

		sides[i] = dists[i] > PLANE_EPSILON ? SIDE_FRONT :
		           dists[i] < PLANE_EPSILON ? SIDE_BACK  : SIDE_ON;

		counts[sides[i]]++;
	}

	sides[i] = sides[0];
	dists[i] = dists[0];

	if((keep_on && !counts[0] && !counts[1]) || !counts[1]) {
		return;
	}

	if(!counts[0]) {
		winding_free(winding);
		return;
	}

	num_points = 0;
	for(i = 0; i < winding->points.items; i++) {
		if(sides[i] == SIDE_ON) {
			++num_points;
			continue;
		}
		if(sides[i] == SIDE_FRONT) {
			++num_points;
		}
		if(sides[i+1] == SIDE_ON || sides[i+1] == sides[i]) {
			continue;
		}
		++num_points;
	}

	array_init(points);
	array_resize(points, num_points);
	for(i = 0; i < winding->points.items; i++) {
		p1 = winding->points.item[i];

		if(sides[i] == SIDE_ON) {
			vec3_copy(p1, points.item[num_points++]);
			++num_points;
			continue;
		}

		if(sides[i] == SIDE_FRONT) {
			vec3_copy(p1, points.item[num_points++]);
		}

		if (sides[i+1] == SIDE_ON || sides[i+1] == sides[i])
			continue;

		// generate a split point
		p2 = winding->points.item[(i + 1) % winding->points.items];

		dot = dists[i] / (dists[i] - dists[i+1]);
		for(j = 0; j < 3; j++) {
			// avoid round off error when possible
			if(plane->normal[j] == 1) {
				mid[j] = plane->dist;
			} else if(plane->normal[j] == -1) {
				mid[j] = -plane->dist;
			} else {
				mid[j] = p1[j] + dot * (p2[j] - p1[j]);
			}
		}

		vec3_copy(mid, points.item[num_points++]);
	}

	array_free(winding->points);
	winding->points.items = points.items;
	winding->points.item = points.item;
}

void winding_print(const struct winding *winding) {
	unsigned int i;

	user_out("-------------\n");
	for(i = 0; i < winding->points.items; i++) {
		user_out("(%5.2f, %5.2f, %5.2f)\n", winding->points.item[i][0], winding->points.item[i][1], winding->points.item[i][2]);
	}
}

void winding_free(struct winding *winding) {
	array_free(winding->points);
}
