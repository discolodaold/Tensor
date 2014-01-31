#include <stdlib.h>
#include <string.h>

#include "str.h"

static unsigned int _hash(const char *s) {
    unsigned int hash = 0xDEADBEEF;
    int c;
    const char *str = s;

    while(c = *str++)
        hash = c + (hash << 6) + (hash << 16) - hash;

    return hash;
}

static int _same(const char *a, const char *b) {
	while(*a && *a++ == *b++) ;
	return *a == *b;
}

struct str2ptr_pair {
	unsigned int hash;
	const char   *key;
	void         *value;
};

struct str2ptr {
	unsigned int        mask;
	unsigned int        fill;
	struct str2ptr_pair *pair;
};

struct str2ptr *new_str2ptr(void) {
	struct str2ptr *result;
	result = calloc(sizeof(*result), 1);
	result->mask = 7;
	result->pair = calloc(sizeof(*result->pair), 8);
	return result;
}

static unsigned int _get_index(struct str2ptr *map, const char *key, unsigned int hash) {
	unsigned int offset = 0, iter = map->mask ? map->mask + 1 : 0;
	while(iter--) {
		unsigned int index = (hash + offset) & map->mask;
		unsigned int entry_hash = map->pair[index].hash;
		if(entry_hash == 0) {
			return map->mask + 1;
		}
		if(hash == entry_hash && _same(key, map->pair[index].key)) {
			return index;
		}
		++offset;
	}
	return map->mask + 1;
}

static void _insert(struct str2ptr *map, const char *key, unsigned int hash, void *value) {
	unsigned int offset = 0, iter = map->mask ? map->mask + 1 : 0;
	while(1) {
		unsigned int index = (hash + offset) & map->mask;
		if(map->pair[index].hash == 0) {
			map->pair[index].hash = hash;
			map->pair[index].key = key;
			map->pair[index].value = value;
			++map->fill;
			return;
		}
		++offset;
	}
}

void *str2ptr_set(struct str2ptr *map, const char *key, void *value) {
	unsigned int hash = _hash(key), index = _get_index(map, key, hash);
	void *old = NULL;
	if(index <= map->mask) {
		old = map->pair[index].value;
		map->pair[index].value = value;
	} else if(map->fill == map->mask + 1) {
		struct str2ptr new_map;
		unsigned int size = (map->mask + 1) << 2;

		memset(&new_map, 0, sizeof(new_map));
		new_map.mask = size - 1;
		new_map.pair = calloc(sizeof(*new_map.pair), size);

		for(index = 0; index < map->mask + 1; ++index) {
			_insert(&new_map, map->pair[index].key, map->pair[index].hash, map->pair[index].value);
		}
		_insert(&new_map, string_dup(key), hash, value);

		free(map->pair);
		*map = new_map;
	} else {
		_insert(map, string_dup(key), hash, value);
	}
	return old;
}

void *str2ptr_get(struct str2ptr *map, const char *key) {
	unsigned int hash = _hash(key), index = _get_index(map, key, hash);
	return (index <= map->mask) ? map->pair[index].value : NULL;
}

void *str2ptr_del(struct str2ptr *map, const char *key) {
	unsigned int hash = _hash(key), index = _get_index(map, key, hash);
	void *result = NULL;
	if(index <= map->mask) {
		result = map->pair[index].value;
		free((char *)map->pair[index].key);
		memset(map->pair + index, 0, sizeof(*map->pair));
	}
	return result;
}

void *str2ptr_pop(struct str2ptr *map, const char **keyPtr) {
	unsigned int iter = map->mask ? map->mask + 1 : 0;
	void *result = NULL;

	while(iter--) {
		if(map->pair[iter].hash != 0) {
			if(keyPtr != NULL) {
				*keyPtr = map->pair[iter].key;
			}
			result = map->pair[iter].value;
			memset(map->pair + iter, 0, sizeof(*map->pair));
			break;
		}
	}

	return result;
}

void *str2ptr_each(struct str2ptr *map, const char **keyPtr) {
	unsigned int hash, index;
	void *result = NULL;

	if(*keyPtr != NULL) {
		hash = _hash(*keyPtr);
		index = _get_index(map, *keyPtr, hash) + 1;
	} else {
		index = 0;
	}

	while(index <= map->mask) {
		if(map->pair[index].hash != 0) {
			*keyPtr = map->pair[index].key;
			result = map->pair[index].value;
			break;
		}
		index++;
	}

	return result;
}

unsigned int str2ptr_size(struct str2ptr *map) {
    return map->fill;
}

void str2ptr_free(struct str2ptr *map) {
	while(str2ptr_pop(map, NULL) != NULL) ;

	if(map->pair) {
		free(map->pair);
	}
	free(map);
}
