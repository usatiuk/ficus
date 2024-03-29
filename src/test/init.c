#include "syscalls_interface.h"

#include "stdio.h"
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/unistd.h>

int main() {
    //    sputchar('h');
    //    sputchar('i');
    //    sputchar('\n');
    uint64_t    test123 = open("/test123", O_CREAT | O_RDWR);
    const char *teststr = "test str";
    write(test123, teststr, 9);
    close(test123);

    test123 = open("/test123", O_RDONLY);
    char buf[123];
    read(test123, buf, 9);

    sputchar('\n');
    for (int i = 0; i < 8; i++) {
        sputchar(buf[i]);
    }
    sputchar('\n');

    usleep(100);

    execve("/hello2", 0, 0);

    while (1) {
        //        sputchar('h');
        //        sputchar('i');
        sputchar('\n');
        char read = sreadchar();
        if (read == 'm') print_mem();
        if (read == 't') print_tasks();
        if (read == 'h') execve("/hello2", 0, 0);
        sputchar('\n');
        sputchar(read);
        //        sleep(100000);
    }
}