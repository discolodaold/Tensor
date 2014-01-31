#ifndef CORE_STR2PTR_H
#define CORE_STR2PTR_H

// hashmap of a string to a pointer
// seems to be a very common structure required by quake

struct str2ptr { unsigned int mask; };

struct str2ptr *new_str2ptr(void);
void *str2ptr_set(struct str2ptr *, const char *, void *);
void *str2ptr_get(struct str2ptr *, const char *);
void *str2ptr_del(struct str2ptr *, const char *);
void *str2ptr_pop(struct str2ptr *, const char **);
void *str2ptr_each(struct str2ptr *, const char **);
unsigned int str2ptr_size(struct str2ptr *);
void str2ptr_free(struct str2ptr *map);

#endif
