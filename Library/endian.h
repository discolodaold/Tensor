#ifndef CORE_ENDIAN_H
#define CORE_ENDIAN_H

#include "scalar.h"

u2 endian_swap_16(u2);
u4 endian_swap_32(u4);

#ifdef LITTLE_ENDIAN
#define endian_little_short(x) (x)
#define endian_big_short(x) (endian_swap_16(x))

#define endian_little_long(x) (x)
#define endian_big_long(x) (endian_swap_32(x))

#define endian_little_float(x) (x)
#define endian_big_float(x) (*(float *)endian_swap_32(*(int *)(x)))
#else
#define endian_little_short(x) (endian_swap_16(x))
#define endian_big_short(x) (x)

#define endian_little_long(x) (endian_swap_32(x))
#define endian_big_long(x) (x)

#define endian_little_float(x) (*(float *)endian_swap_32(*(int *)(x)))
#define endian_big_long(x) (x)
#endif

#endif
