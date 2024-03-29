#include "syscalls_interface.h"

volatile char        asdfasdf[323];
volatile int         x     = 3;
volatile int         w     = 0;

volatile const char *hello = "hello xd";

int                  main() {
    if (x == 3) putchar('x');
    if (w == 2) putchar('w');
    if (asdfasdf[0] == '\0') putchar('a');
    putchar('h');
    putchar('i');
    putchar('\n');
}