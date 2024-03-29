#include "syscalls_interface.h"


int main() {
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

    sleep(100);

    execve("/hello2", 0, 0);

    while (1) {
        //        putchar('h');
        //        putchar('i');
        putchar('\n');
        char read = readchar();
        if (read == 'm') print_mem();
        if (read == 't') print_tasks();
        if (read == 'h') execve("/hello2", 0, 0);
        putchar('\n');
        putchar(read);
        //        sleep(100000);
    }
}