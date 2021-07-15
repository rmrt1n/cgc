#include <stdio.h>
#include <string.h>
#include "gc.h"

char *test() {
    char *s = gc_alloc(100);
    return s;
}

int main(int argc, char **argv) {
    (void)argv; // to supress -Wunused-parameter
    gc_start(&argc); 
    char *s = test();
    strcpy(s, "hello");
    puts(s);
    gc_stop();
    return 0;
}

