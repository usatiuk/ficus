#include "syscalls_interface.h"

void _start() {
    while (true) {
        putchar('h');
        putchar('i');
        putchar('\n');
        sleep(100000);
    }
}