#include <stdio.h>
#include <sys/unistd.h>


int main() {
    printf("Hi!\n");
    if (fork() == 0) {
        printf("I'm a fork!\n");
    } else {
        printf("Forked!\n");
    }
}