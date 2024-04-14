#include <stdio.h>
#include <sys/unistd.h>
#include <sys/wait.h>


int main() {
    printf("Hi!\n");
    if (fork() == 0) {
        printf("I'm a fork!\n");
        sleep(100);
        printf("I'm exiting!\n");
        return 1;
    } else {
        printf("Forked!\n");
        wait(NULL);
        printf("Fork exited!\n");
    }

    if (fork() == 0) {
        printf("I'm a fork!\n");
        printf("I'm exiting!\n");
        return 1;
    } else {
        printf("Forked!\n");
        sleep(100);
        wait(NULL);
        printf("Fork exited!\n");
    }

    pid_t forkpid = fork();
    if (forkpid == 0) {
        printf("I'm a fork!\n");
        printf("I'm exiting!\n");
        return 1;
    } else {
        printf("Forked!\n");
        sleep(100);
        waitpid(forkpid, NULL, 0);
        printf("Fork exited!\n");
    }

    forkpid = fork();
    if (forkpid == 0) {
        printf("I'm a fork!\n");
        sleep(100);
        printf("I'm exiting!\n");
        return 1;
    } else {
        printf("Forked!\n");
        waitpid(forkpid, NULL, 0);
        printf("Fork exited!\n");
    }
}