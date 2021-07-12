#include <stdio.h>
#include "gc.h"

void test() {
    char *s = gc_alloc(100);
    int *d = gc_alloc(5);
    int *e = gc_alloc(3);
}

int main(int argc, char **argv) {
    gc_start(&argc); 
    test();
    gc_stop();
    return 0;
}

