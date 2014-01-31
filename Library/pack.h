#ifndef CORE_PACK_H
#define CORE_PACK_H

#include "mem.h"

struct pack;

struct pack *load_pack(const char *, FILE *);
const char *pack_filename(struct pack *);
unsigned int pack_num_entries(struct pack *);
int pack_get(struct pack *, const char *, const unsigned char **);
int pack_get_file(struct pack *, const char *, FILE **file);
void pack_set(struct pack *, const char *, unsigned int, const unsigned char *);
void pack_del(struct pack *, const char *);
void pack_save(struct pack *, FILE *);
void pack_free(struct pack *);

#endif
