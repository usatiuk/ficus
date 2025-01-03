/* note these headers are all provided by newlib - you don't need to provide them */
#include <dirent.h>
#include <stdio.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/syscalls.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/wait.h>

uint64_t _do_syscall(uint64_t id_rdi, uint64_t a1_rsi, uint64_t a2_rdx, uint64_t a3_rcx) {
    register uint64_t res asm("rax");
    if (id_rdi != SYSCALL_FORK_ID)
        asm volatile("syscall; mov (0x10010), %%rsp;" // TASK_POINTER->ret_sp_val
                     : "=ra"(res)
                     : "D"(id_rdi), "S"(a1_rsi), "d"(a2_rdx), "a"(a3_rcx)
                     : "cc", "rcx", "r8",
                       "r9", "r10", "r11", "r15", "memory");
    else
        asm volatile("syscall; mov (0x10010), %%rsp;" // TASK_POINTER->ret_sp_val
                     "pop %%r15;"
                     "pop %%r14;"
                     "pop %%r13;"
                     "pop %%r12;"
                     "pop %%rbp;"
                     "pop %%rbx;"
                     : "=ra"(res)
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

char **environ = 0; /* pointer to array of char * strings that define the current environment variables */

int _execve(char *name, char **argv, char **env) {
    return _do_syscall(SYSCALL_EXECVE_ID, (uint64_t) name, (uint64_t) argv, (uint64_t) env);
}

int _fork() {
    return _do_syscall(SYSCALL_FORK_ID, 0, 0, 0);
}

int _getpid() {
    return -1;
}

int _isatty(int file) { return file == 0 || file == 1 || file == 2; }

int _fstat(int file, struct stat *st) {
    if (_isatty(file)) st->st_mode = S_IFCHR;
}

int _kill(int pid, int sig) {
    return -1;
}

int _link(char *old, char *new) {
    return -1;
}

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

int _stat(const char *file, struct stat *st) {
    return -1;
}

clock_t _times(struct tms *buf) {
    buf->tms_cstime = 0;
    buf->tms_cutime = 0;
    buf->tms_stime  = 0;
    buf->tms_utime  = 0;
    return 0;
}

int _unlink(char *name) {
    return -1;
}

int _wait(int *status) {
    return waitpid(-1, &status, 0);
}

pid_t waitpid(pid_t pid, int *status, int options) {
    return _do_syscall(SYSCALL_WAITPID_ID, (uint64_t) pid, (uint64_t) status, (uint64_t) options);
}

int _write(int file, char *ptr, int len) {
    return _do_syscall(SYSCALL_WRITE_ID, file, (uint64_t) ptr, len);
}

int getdents(int fd, struct dirent *dp, int count) {
    return _do_syscall(SYSCALL_READDIR_ID, fd, (uint64_t) dp, count);
}

int sleep(int seconds) {
    return _do_syscall(SYSCALL_SLEEP_ID, seconds * 1000, 0, 0);
}

int usleep(useconds_t useconds) {
    return _do_syscall(SYSCALL_SLEEP_ID, useconds, 0, 0);
}


int _gettimeofday(struct timeval *restrict p, void *restrict z) {
    p->tv_sec  = 0;
    p->tv_usec = 0;
    return 0;
}

void print_mem() {
    _do_syscall(SYSCALL_PRINT_MEM, 0, 0, 0);
}
void print_tasks() {
    _do_syscall(SYSCALL_PRINT_TASKS, 0, 0, 0);
}
