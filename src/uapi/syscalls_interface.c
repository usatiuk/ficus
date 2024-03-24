//
// Created by Stepan Usatiuk on 26.10.2023.
//

#include "syscalls_interface.h"
#include "syscalls_defs.h"


uint64_t do_syscall(uint64_t id_rdi, uint64_t a1_rsi, uint64_t a2_rdx, uint64_t a3_rcx) {
    uint64_t res;
    asm volatile("syscall; mov (0x10016), %%rsp" // TASK_POINTER->ret_sp_val
                 : "=r"(res)
                 : "D"(id_rdi), "S"(a1_rsi), "d"(a2_rdx), "a"(a3_rcx)
                 : "cc", "rcx", "r8",
                   "r9", "r10", "r11", "r15", "memory");
    return res;
}

void exit() {
    do_syscall(SYSCALL_EXIT_ID, 0, 0, 0);
}

uint64_t readchar() {
    return do_syscall(SYSCALL_READCHAR_ID, 0, 0, 0);
}

uint64_t putchar(char c) {
    return do_syscall(SYSCALL_PUTCHAR_ID, c, 0, 0);
}

uint64_t sleep(uint64_t micros) {
    return do_syscall(SYSCALL_SLEEP_ID, micros, 0, 0);
}

uint64_t open(const char *pathname, int flags) {
    return do_syscall(SYSCALL_OPEN_ID, (uint64_t) pathname, flags, 0);
}

uint64_t close(uint64_t FD) {
    return do_syscall(SYSCALL_CLOSE_ID, FD, 0, 0);
}

uint64_t read(uint64_t fd, char *buf, uint64_t len) {
    return do_syscall(SYSCALL_READ_ID, fd, (uint64_t) buf, len);
}

uint64_t write(uint64_t fd, const char *buf, uint64_t len) {
    return do_syscall(SYSCALL_WRITE_ID, fd, (uint64_t) buf, len);
}

uint64_t lseek(uint64_t fd, uint64_t off, uint64_t whence) {
    return do_syscall(SYSCALL_LSEEK_ID, fd, off, whence);
}

uint64_t execve(const char *pathname, char *const argv[], char *const envp[]) {
    return do_syscall(SYSCALL_EXECVE_ID, (uint64_t) pathname, (uint64_t) argv, (uint64_t) envp);
}

void print_mem() {
    do_syscall(SYSCALL_PRINT_MEM, 0, 0, 0);
}
void print_tasks() {
    do_syscall(SYSCALL_PRINT_TASKS, 0, 0, 0);
}
