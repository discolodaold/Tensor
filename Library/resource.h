#ifndef CORE_RESOURCE_H
#define CORE_RESOURCE_H

#include <stdlib.h>

struct resource {
    void *(*load)(size_t, void *);
    void (*release)(void *);
    void (*refresh)(void *);
};

int resource_init(void);
void resource_load(const struct resource * restrict resource, const char * restrict filename, void ** restrict ptr);
void resource_barrier(void);
void resource_deinit(void);

#endif
