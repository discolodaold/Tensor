#ifndef MEM_H
#define MEM_H

#include <string.h>
#include <malloc.h>

int mem_init();

#define mem_alloc malloc
#define mem_free(X) free((void *)X)

#define mem_copy(DST, SRC, SIZE) memcpy(DST, SRC, SIZE)

#endif
