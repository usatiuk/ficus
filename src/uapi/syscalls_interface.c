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

uint64_t sreadchar() {
    return do_syscall(SYSCALL_READCHAR_ID, 0, 0, 0);
}

uint64_t sputchar(char c) {
    return do_syscall(SYSCALL_PUTCHAR_ID, c, 0, 0);
}

void print_mem() {
    do_syscall(SYSCALL_PRINT_MEM, 0, 0, 0);
}
void print_tasks() {
    do_syscall(SYSCALL_PRINT_TASKS, 0, 0, 0);
}
