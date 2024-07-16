#include "sys/syscalls.h"

#include "stdio.h"
#include "stdlib.h"
#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

void ls(char *dname) {
    while (dname[0] == ' ' && dname[0] != '\0') dname++;
    DIR *rfd = opendir(dname);
    if (rfd == NULL) {
        printf("Unknown directory: %s\n", dname);
        return;
    }
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
        printf("\n> ");
        char line[30];
        scanf(" %24[^\n]", line);
        if (strncmp(line, "ls", 2) == 0) {
            ls(line + 2);
        } else if (strcmp(line, "mem") == 0) {
            print_mem();
        } else if (strcmp(line, "tasks") == 0) {
            print_tasks();
        } else {
            if (fork() == 0) {
                execve(line, 0, 0);
                printf("Failed to start: %s\n", line);
                return 0;
            } else
                wait(NULL);
        }
    }
}