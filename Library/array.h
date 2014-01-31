#ifndef ARRAY_H
#define ARRAY_H

#include "mem.h"

#define array(TYPE) struct { size_t items; TYPE *item; }
#define array_init(ARRAY) do { (ARRAY).items = 0; (ARRAY).item = NULL; } while(0)
#define array_resize(ARRAY, NEWSIZE) do { (ARRAY).items = (NEWSIZE); *((void **)&(ARRAY).item) = realloc((ARRAY).item, sizeof(*(ARRAY).item) * ((ARRAY).items)); } while(0)
#define array_append(ARRAY, NEWITEM) do { array_resize(ARRAY, (ARRAY).items + 1); (ARRAY).item[(ARRAY).items - 1] = NEWITEM; } while(0)
#define array_for(ARRAY, NEEDLE) for(NEEDLE = (ARRAY).item; NEEDLE < (ARRAY).item + (ARRAY).items; ++NEEDLE)
#define array_for_reversed(ARRAY, NEEDLE) for(NEEDLE = (ARRAY).item + (ARRAY).items - 1; NEEDLE >= (ARRAY).item; --NEEDLE)
#define array_index(ARRAY, ITEM, CMP) ({ int _i = 0, _r = -1; typeof(ITEM) _item = ITEM, _needle; typeof(CMP) _cmp = CMP; for(; _i < (ARRAY).items; ++_i) { if(_cmp(_item, (ARRAY).item[_i]) == 0) { _r = _i; break; } } _r; })
#define array_free(ARRAY) do { free((ARRAY).item); (ARRAY).items = 0; (ARRAY).item = NULL; } while(0)
#define array_free_each(ARRAY, FN) do { typeof((ARRAY).item) _needle; array_for(ARRAY, _needle) { FN(*_needle); } array_free(ARRAY); } while(0)

#define array_type(ARRAY) typeof(*(ARRAY).item)
#define array_sort(ARRAY, FN) qsort((ARRAY).item, (ARRAY).items, sizeof(*(ARRAY).item), (int (*)(const void *, const void *))FN)
#define array_sort_cb(ARRAY) auto int __LINE__ ## comparator(array_type(ARRAY) *, array_type(ARRAY) *); array_sort(ARRAY, __LINE__ ## comparator); int __LINE__ ## comparator(array_type(ARRAY) *a, array_type(ARRAY) *b)

#endif
