#include "syscalls_interface.h"

void _start() {
    //    putchar('h');
    //    putchar('i');
    //    putchar('\n');
    while (true) {
        //        putchar('h');
        //        putchar('i');
        //        putchar('\n');
        putchar(readchar());
        //        sleep(100000);
    }
}