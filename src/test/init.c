#include "syscalls_interface.h"

#include "stdio.h"
#include "stdlib.h"
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/unistd.h>


int main() {
    malloc(100);
    malloc(100);
    malloc(100);
    malloc(100);
    printf("Who are you?\n");
    char  *line = NULL;
    size_t len  = 0;
    //    __getline(&line, &len, stdin);


    printf("hi %s\n", line);
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

    printf("\n %s \n", buf);

    usleep(100);

    execve("/hello2", 0, 0);

    while (1) {
        printf("\n");
        char c;
        scanf(" %c", &c);
        if (c == 'm') print_mem();
        if (c == 't') print_tasks();
        if (c == 'h') execve("/hello2", 0, 0);
        printf("%c", c);
    }
}