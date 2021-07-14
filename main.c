#include <stdio.h>
#include <string.h>
#include "gc.h"

char *test() {
    char *s = gc_alloc(100);
    return s;
}

int main(int argc, char **argv) {
    gc_start(&argc); 
    char *s = test();
    printf("addr of s %p\n", s);
    // strcpy(s, "hello");
    // puts(s);
    gc_stop();
    return 0;
}

