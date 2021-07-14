#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "gc.h"

GarbageCollector GLOBAL_GC;

static void gc_ht_set(void *ptr, size_t size);
static Allocd *gc_ht_get(void *ptr);
static void gc_ht_del(void *ptr);

void gc_start(void *bos) {
    GLOBAL_GC = (GarbageCollector){
        .bos = bos,
        .size = GC_BASE_SIZE,
        .used = 0,
        .limit = GC_BASE_LIMIT,
        .stored = 0,
        .ptrs = calloc(GC_BASE_SIZE, sizeof(Allocd *))
    };
}

void gc_stop() {
    gc_run();
    for (unsigned int i = 0; i < GLOBAL_GC.size; i++) {
        Allocd *cur = GLOBAL_GC.ptrs[i];
        while (cur != NULL) {
            Allocd *tmp = cur;
            cur = cur->next;
            free(tmp);
        }
    }
    free(GLOBAL_GC.ptrs);
}

static int is_marked(Allocd *a) {
    return a->marked ^ 0; // 1 ^ 0 is true
}

static void gc_mark(void *start, void *end) {
    uintptr_t p = (uintptr_t)start;
    for (; p < (uintptr_t)end; p++) {
            printf("search %p\n", *(void **)p);
        Allocd *cur = gc_ht_get(*((void **)p));
        if (cur != NULL && !is_marked(cur)) {
            cur->marked = 1;
            gc_mark(cur->ptr, cur->ptr + cur->size);
        }
    }
}

static void gc_sweep() {
    for (unsigned int i = 0; i < GLOBAL_GC_SIZE; i++) {
        Allocd *cur = GLOBAL_GC_PTRS[i];
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
    char tos;
    gc_mark(&tos, GLOBAL_GC_BOS);
    gc_sweep();
}

static Allocd *allocd_new(void *ptr, size_t size) {
    Allocd *a = malloc(sizeof(Allocd));
    a->ptr = ptr;
    a->size = size;
    a->marked = 0;
    a->next = NULL;
    return a;
}

static void gc_ht_resize(int new_size) {
    if (new_size < GC_BASE_SIZE) return;
    
    int old_size = GLOBAL_GC_SIZE;
    GLOBAL_GC_SIZE = new_size;
    GLOBAL_GC_USED = 0;
    Allocd **old_ptrs = GLOBAL_GC_PTRS;
    GLOBAL_GC_PTRS = calloc(new_size, sizeof(Allocd *));

    for (int i = 0; i < old_size; i++) {
        Allocd *cur = old_ptrs[i];
        while (cur != NULL) {
            gc_ht_set(cur->ptr, cur->size);
            cur = cur->next;
        }
    }
    free(old_ptrs);
}

static void gc_ht_resize_up() {
    int load = GLOBAL_GC_USED / GLOBAL_GC_SIZE * 100;
    if (load > 70) gc_ht_resize(GLOBAL_GC_SIZE * 2);
}

static void gc_ht_resize_down() {
    int load = GLOBAL_GC_USED / GLOBAL_GC_SIZE * 100;
    if (load < 10) gc_ht_resize(GLOBAL_GC_SIZE / 2);
}

static void gc_ht_set(void *ptr, size_t size) {
    Allocd *new = allocd_new(ptr, size);
    int hash = (uintptr_t)ptr % GLOBAL_GC_SIZE;
    Allocd *cur = GLOBAL_GC_PTRS[hash], *prev = NULL;
    while (cur != NULL) {
        if (cur->ptr == ptr) {
            new->next = cur->next;
            if (prev == NULL) {
                GLOBAL_GC_PTRS[hash] = new;
            } else {
                prev->next = new;
            }
        }
        prev = cur;
        cur = cur->next;
    }
    cur = GLOBAL_GC_PTRS[hash];
    new->next = cur;
    GLOBAL_GC_PTRS[hash] = new;
    GLOBAL_GC_USED++;
    gc_ht_resize_up();
}

static Allocd *gc_ht_get(void *ptr) {
    int hash = (uintptr_t)ptr % GLOBAL_GC_SIZE;
    Allocd *cur = GLOBAL_GC_PTRS[hash];
    while (cur != NULL) {
        if (cur->ptr == ptr) return cur;
        cur = cur->next;
    }
    return NULL;
}

static void gc_ht_del(void *ptr) {
    int hash = (uintptr_t)ptr % GLOBAL_GC_SIZE;
    Allocd *cur = GLOBAL_GC_PTRS[hash], *prev = NULL;
    while (cur != NULL) {
        if (cur->ptr == ptr) {
            if (prev == NULL) {
                GLOBAL_GC_PTRS[hash] = cur->next;
            } else {
                prev->next = cur->next;
            }
            free(cur);
            GLOBAL_GC_USED--;
            gc_ht_resize_down();
            break;
        }
        prev = cur;
        cur = cur->next;
    }
}

void *gc_alloc(size_t size) {
    // if reached gc allocd limit
    if (GLOBAL_GC_STORED > GLOBAL_GC_LIMIT) {
        GLOBAL_GC_LIMIT = GLOBAL_GC_STORED * 1.5;
        gc_run();
    } 
    void *ptr = malloc(size);
    if (ptr == NULL) {
        gc_run();
        ptr = malloc(size);
        if (ptr == NULL) return NULL;
    }
    gc_ht_set(ptr, size);
    return ptr;
}

