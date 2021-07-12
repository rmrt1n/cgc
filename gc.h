#ifndef GC_H
#define GC_H

#include <stddef.h>

#define GC_BASE_SIZE 8
#define PTRSIZE (sizeof(char *))

typedef struct Allocd {
    void *ptr;
    size_t size;
    char marked;
    struct Allocd *next;
} Allocd;

typedef struct GarbageCollector {
    void *bos;
    int size;
    int used;
    Allocd **ptrs;
} GarbageCollector;

extern GarbageCollector GLOBAL_GC;

// gc funcs
void gc_start(void *bos);
void gc_stop(void);
void gc_run(void);

// allocator
void *gc_alloc(size_t size);

#endif

