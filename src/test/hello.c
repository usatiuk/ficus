#include "syscalls_interface.h"


void _start() {
    //    putchar('h');
    //    putchar('i');
    //    putchar('\n');
    uint64_t    test123 = open("/test123", O_CREAT | O_RDWR);
    const char *teststr = "test str";
    write(test123, teststr, 9);
    close(test123);

    test123 = open("/test123", O_RDONLY);
    char buf[123];
    read(test123, buf, 9);

    putchar('\n');
    for (int i = 0; i < 8; i++) {
        putchar(buf[i]);
    }
    putchar('\n');

    while (1) {
        //        putchar('h');
        //        putchar('i');
        //        putchar('\n');
        putchar(readchar());
        //        sleep(100000);
    }
}