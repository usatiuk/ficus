/* note these headers are all provided by newlib - you don't need to provide them */
#include <stdio.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>

#define SYSCALL_EXIT_ID     0

#define SYSCALL_PUTCHAR_ID  1
#define SYSCALL_SLEEP_ID    2
#define SYSCALL_READCHAR_ID 3

#define SYSCALL_OPEN_ID     4
#define SYSCALL_CLOSE_ID    5

#define SYSCALL_READ_ID     6
#define SYSCALL_WRITE_ID    7
#define SYSCALL_LSEEK_ID    8

#define SYSCALL_OPENDIR_ID  9
#define SYSCALL_READDIR_ID  10
#define SYSCALL_CLOSEDIR_ID 11
#define SYSCALL_MKDIR_ID    12
#define SYSCALL_UNLINK_ID   13

#define SYSCALL_EXECVE_ID   50
#define SYSCALL_SBRK_ID     100


#define SYSCALL_PRINT_MEM   1000
#define SYSCALL_PRINT_TASKS 1001

uint64_t _do_syscall(uint64_t id_rdi, uint64_t a1_rsi, uint64_t a2_rdx, uint64_t a3_rcx) {
    uint64_t res;
    asm volatile("syscall; mov (0x10016), %%rsp" // TASK_POINTER->ret_sp_val
            : "=r"(res)
            : "D"(id_rdi), "S"(a1_rsi), "d"(a2_rdx), "a"(a3_rcx)
            : "cc", "rcx", "r8",
    "r9", "r10", "r11", "r15", "memory");
    return res;
}

void _exit() {
    _do_syscall(SYSCALL_EXIT_ID, 0, 0, 0);
}

int _close(int file) {
    return _do_syscall(SYSCALL_CLOSE_ID, file, 0, 0);
}

char **environ; /* pointer to array of char * strings that define the current environment variables */
int _execve(char *name, char **argv, char **env) {
    return _do_syscall(SYSCALL_EXECVE_ID, (uint64_t) name, (uint64_t) argv, (uint64_t) env);
}

int _fork() {}

int _getpid() {}

int _isatty(int file) { return file == 0 || file == 1 || file == 2; }

int _fstat(int file, struct stat *st) {
    if (_isatty(file)) st->st_mode = S_IFCHR;
}

int _kill(int pid, int sig) {}

int _link(char *old, char *new) {}

int _lseek(int file, int ptr, int dir) {
    return _do_syscall(SYSCALL_LSEEK_ID, file, ptr, dir);
}

int _open(const char *name, int flags, ...) {
    return _do_syscall(SYSCALL_OPEN_ID, (uint64_t) name, flags, 0);
}

int _read(int file, char *ptr, int len) {
    return _do_syscall(SYSCALL_READ_ID, file, (uint64_t) ptr, len);
}

caddr_t _sbrk(int incr) {
    return (caddr_t) _do_syscall(SYSCALL_SBRK_ID, (int64_t) incr, 0, 0);
}

int _stat(const char *file, struct stat *st) {}

clock_t _times(struct tms *buf) {}

int _unlink(char *name) {}

int _wait(int *status) {}

int _write(int file, char *ptr, int len) {
    return _do_syscall(SYSCALL_WRITE_ID, file, (uint64_t) ptr, len);
}

int sleep(int seconds) {
    return _do_syscall(SYSCALL_SLEEP_ID, seconds * 1000, 0, 0);
}

int usleep(useconds_t useconds) {
    return _do_syscall(SYSCALL_SLEEP_ID, useconds, 0, 0);
}


int _gettimeofday(struct timeval *restrict p, void *restrict z) {}