#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <pthread.h>
#include <semaphore.h>

#include "resource.h"
#include "str.h"
#include "fs.h"
#include "mem.h"

//{{{ A Fix for the ABA problem
#if 0
struct aba {
    void *ptr;
    uintptr_t tag;
};

static int aba_update(struct aba *old, void *ptr) {
    struct aba new = {
        .ptr = ptr,
        .tag = old->tag + 1
    };
    unsigned char result;
#if _WORDSIZE == 64
    __asm__ __volatile__ (
        "lock cmpxchg16b oword ptr %1\n"
        "setz %0\n"
        : "=q"(result), "+m"(*old), "+d"(old->ptr), "+a"(old->tag)
        : "c"(new.ptr), "b"(new.tag)
        : "cc"
    );
    return result;
#else
    __asm__ __volatile__ (
        "lock cmpxchg8b qword %1\n"
        "setz %0\n"
        : "=q"(result), "+m"(*old), "+d"(old->ptr), "+a"(old->tag)
        : "c"(new.ptr), "b"(new.tag)
        : "cc"
    );
    return result;
#endif
}
#endif
//}}}

//{{{ Messages
struct loaderMessage {
    enum {
        MSG_LOAD,
        MSG_BARRIER,
        MSG_EXIT
    } tag;
    char *filename;
	const struct resource *resource;
    void **ptr;
};

// _head belongs to consumers (single)
static uint32_t _read = 0;

// _tail belongs to producers (multiple)
static uint32_t _write = 0;

static sem_t _empty_count;
static sem_t _full_count;

enum {
    MESSAGE_COUNT = 32
};
struct loaderMessage _messages[MESSAGE_COUNT];

// multiple producers
static void _enqueue(struct loaderMessage *m) {
    sem_wait(&_empty_count);
    int write = __sync_fetch_and_add(&_write, 1);
    _messages[write & (MESSAGE_COUNT - 1)] = *m;
    sem_post(&_full_count);
}

// single consumer
static void _dequeue(struct loaderMessage *m) {
    sem_wait(&_full_count);
    int read = __sync_fetch_and_add(&_read, 1);
    *m = _messages[read & (MESSAGE_COUNT - 1)];
    sem_post(&_empty_count);
}
//}}}

struct _resource {
    struct _resource *next;
	const struct resource *resource;
    char *filename;
    void *data;
    unsigned int sequence;
};

static void *_loader(void *unused) {
    FILE *f;
    char *buffer;
    size_t length;
    struct _resource *r, *active_resources = NULL, *free_resources = NULL, *r_next;
    struct loaderMessage m;
    unsigned int sequence = 0;

    while(1) {
delt_with:
        _dequeue(&m);
        if(m.tag == MSG_EXIT) {
            r = active_resources;
            while(r) {
                r_next = r->next;
                r->resource->release(r->data);
#ifdef DEBUG
                free(r->filename);
#endif
                r->next = free_resources;
                free_resources = r;
                r = r_next;
            }
#ifdef DEBUG
            r = free_resources;
            while(r) {
                r_next = r->next;
                free(r);
                r = r_next;
            }
#endif
            return NULL;
        }

        if(m.tag == MSG_BARRIER) {
            r = active_resources;
            active_resources = NULL;
            while(r) {
                r_next = r->next;
                if(r->sequence != sequence) {
                    r->resource->release(r->data);
                    free(r->filename);
                    r->next = free_resources;
                    free_resources = r;
                } else {
                    r->next = active_resources;
                    active_resources = r;
                }
                r = r_next;
            }
            sequence++;
            continue;
        }

        r = active_resources;
        while(r) {
            if(string_icmp(r->filename, m.filename) == 0) {
                r->sequence = sequence;
				r->resource->refresh(r->data);
                *m.ptr = r->data;
                free(m.filename);
                goto delt_with;
            }
            r = r->next;
        }

        if(free_resources) {
            r = free_resources;
            free_resources = r->next;
        } else {
            r = calloc(1, sizeof(*r));
        }

        length = fs_read(m.filename, (const unsigned char **)&buffer);
        if(length < 0) {
            continue;
        }

        r->data = m.resource->load(length, buffer);
        mem_free(buffer);

        if(r->data) {
            *m.ptr = r->data;
            r->filename = m.filename;
			r->resource = m.resource;
            r->sequence = sequence;

            r->next = active_resources;
            active_resources = r;
        }
    }
}

static pthread_t _resource_thread;

int resource_init(void) {
    int r;

    _read = _write = 0;

    sem_init(&_empty_count, 0, MESSAGE_COUNT);
    sem_init(&_full_count, 0, 0);

    if((r = pthread_create(&_resource_thread, NULL, _loader, NULL))) {
        printf("loader failed to start, %i\n", r);
		return 0;
    }
	
	return 1;
}

void resource_load(const struct resource * restrict resource, const char * restrict filename, void ** restrict ptr) {
    struct loaderMessage m;
    memset(&m, 0, sizeof(m));
    m.tag = MSG_LOAD;
    m.filename = (char *)string_dup(filename);
	m.resource = resource;
    m.ptr = ptr;
    _enqueue(&m);
}

void resource_barrier(void) {
    struct loaderMessage m;
    memset(&m, 0, sizeof(m));
    m.tag = MSG_BARRIER;
    _enqueue(&m);
}

void resource_deinit(void) {
    struct loaderMessage m;
    memset(&m, 0, sizeof(m));
    m.tag = MSG_EXIT;
    _enqueue(&m);
    pthread_join(_resource_thread, NULL);
}
