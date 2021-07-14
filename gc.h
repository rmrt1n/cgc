#ifndef GC_H
#define GC_H

#include <stddef.h>

// gc base values
#define GC_BASE_SIZE 64
#define GC_BASE_LIMIT 128000 // 128kb gc threshold 
// global gc member aliases
#define GLOBAL_GC_BOS GLOBAL_GC.bos
#define GLOBAL_GC_SIZE GLOBAL_GC.size
#define GLOBAL_GC_USED GLOBAL_GC.used
#define GLOBAL_GC_LIMIT GLOBAL_GC.limit
#define GLOBAL_GC_STORED GLOBAL_GC.stored
#define GLOBAL_GC_PTRS GLOBAL_GC.ptrs

typedef struct Allocd {
    void *ptr;
    size_t size : 63;
    char marked : 1;
    struct Allocd *next;
} Allocd;

typedef struct GarbageCollector {
    void *bos;
    unsigned int size, used;
    size_t limit, stored;
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

