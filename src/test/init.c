#include "sys/syscalls.h"

#include "stdio.h"
#include "stdlib.h"
#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

void ls() {
    DIR           *rfd = opendir("/");
    struct dirent *cur = readdir(rfd);
    while (cur) {
        printf("%s\n", cur->d_name);
        cur = readdir(rfd);
    }
    closedir(rfd);
}

int main() {
    {
        uint64_t    test123 = open("/test123", O_CREAT | O_RDWR);
        const char *teststr = "test str";
        write(test123, teststr, 9);
        close(test123);

        test123 = open("/test123", O_RDONLY);
        char buf[123];
        read(test123, buf, 9);
        close(test123);
        printf("\n %s \n", buf);
    }
    while (1) {
        if (fork() == 0)
            execve("hello2", 0, 0);
        else
            wait(NULL);
        print_mem();
        sleep(500);
    }

    while (1) {
        printf("\n> ");
        char line[30];
        scanf(" %24[^\n]", line);
        if (strcmp(line, "ls") == 0) {
            ls();
        } else if (strcmp(line, "mem") == 0) {
            print_mem();
        } else if (strcmp(line, "tasks") == 0) {
            print_tasks();
        } else {
            if (fork() == 0)
                execve(line, 0, 0);
            else
                wait(NULL);
        }
    }
}