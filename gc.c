#include <stdlib.h>
#include <stdint.h>
#include "gc.h"

GarbageCollector GLOBAL_GC;

static Allocd *gc_ht_get(void *ptr);
static void gc_ht_del(void *ptr);

void gc_start(void *bos) {
    GLOBAL_GC = (GarbageCollector){
        .bos = bos,
        .size = GC_BASE_SIZE,
        .used = 0,
        .ptrs = calloc(GC_BASE_SIZE, sizeof(Allocd *))
    };
}

void gc_stop() {
    gc_run();
    for (int i = 0; i < GLOBAL_GC.size; i++) {
        Allocd *cur = GLOBAL_GC.ptrs[i];
        while (cur != NULL) {
            Allocd *tmp = cur;
            free(tmp);
            cur = cur->next;
        }
    }
    free(GLOBAL_GC.ptrs);
}

static int is_marked(Allocd *a) {
    return a->marked == 1;
}

static void gc_mark(void *start, void *end) {
    for (char *p = (char *)start; p < (char *)end - PTRSIZE; p++) {
        Allocd *cur = gc_ht_get((void *)p);
        if (cur != NULL && !is_marked(cur)) {
            cur->marked = 1;
            gc_mark(cur->ptr, cur->ptr + cur->size);
        }
    }
}

static void gc_sweep() {
    for (int i = 0; i < GLOBAL_GC.size; i++) {
        Allocd *cur = GLOBAL_GC.ptrs[i];
        while (cur != NULL) {
            if (is_marked(cur)) {
                cur->marked = 0;
                cur = cur->next;
            } else {
                free(cur->ptr);
                Allocd *tmp = cur->next;
                gc_ht_del(cur->ptr);
                cur = tmp;
            }
        }
    }
}

void gc_run() {
    char top;
    // start from lower address (top) to higher addr (bottom)
    gc_mark(&top, GLOBAL_GC.bos);
    gc_sweep();
}

static Allocd *allocd_new(void *ptr, size_t size) {
    Allocd *new = malloc(sizeof(Allocd));
    new->ptr = ptr;
    new->size = size;
    new->marked = 0;
    new->next = NULL;
    return new;
}

static int hash_func(void *ptr) {
    return (((uintptr_t)ptr) >> 3) % GLOBAL_GC.size;
}

static Allocd *gc_ht_get(void *ptr) {
    int hash = hash_func(ptr);
    Allocd *cur = GLOBAL_GC.ptrs[hash];
    while (cur != NULL) {
        if (cur->ptr == ptr) return cur;
        cur = cur->next;
    }
    return NULL;
}

static void gc_ht_set(void *ptr, size_t size) {
    Allocd *new = allocd_new(ptr, size);
    int hash = hash_func(ptr);
    Allocd *cur = GLOBAL_GC.ptrs[hash], *prev = NULL;
    while (cur != NULL) {
        if (cur->ptr == ptr) {
            new->next = cur->next;
            if (prev == NULL) {
                GLOBAL_GC.ptrs[hash] = new;
            } else {
                prev->next = new;
            }
        }
        prev = cur;
        cur = cur->next;
    }

    cur = GLOBAL_GC.ptrs[hash];
    new->next = cur;
    GLOBAL_GC.ptrs[hash] = new;
    GLOBAL_GC.used++;
}

static void gc_ht_del(void *ptr) {
    int hash = hash_func(ptr);
    Allocd *cur = GLOBAL_GC.ptrs[hash], *prev = NULL;
    while (cur != NULL) {
        if (cur->ptr == ptr) {
            if (prev == NULL) {
                GLOBAL_GC.ptrs[hash] = cur->next;
            } else {
                prev->next = cur->next;
            }
            free(cur);
            GLOBAL_GC.used--;
            break;
        }
        prev = cur;
        cur = cur->next;
    }
}

static int gc_load() {
    return GLOBAL_GC.used * 100 / GLOBAL_GC.size;
}

void *gc_alloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        gc_run();
        ptr = malloc(size);
        if (ptr == NULL) return NULL;
    }
    gc_ht_set(ptr, size);
    if (gc_load() > 60) gc_run();
    return ptr;
}

