#include "scalar.h"

u2 endian_swap_16(u2 v) {
	u1 b1, b2;

	b1 = v & 255;
	b2 = (v >> 8) & 255;

	return ((u2)b1 << 8) + b2;
}

u4 endian_swap_32(u4 v) {
	u1 b1, b2, b3, b4;

	b1 = v & 255;
	b2 = (v >> 8) & 255;
	b3 = (v >> 16) & 255;
	b4 = (v >> 24) & 255;

	return ((u4)b1 << 24) + ((u4)b2 << 16) + ((u4)b3 << 8) + b4;
}
