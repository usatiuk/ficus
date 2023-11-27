//
// Created by Stepan Usatiuk on 26.10.2023.
//

#include "syscalls_interface.h"
#include "syscalls_defs.h"

uint64_t do_syscall(uint64_t num, uint64_t a1_rsi) {
    uint64_t res;
    asm volatile("syscall; mov (0x10016), %%rsp"// TASK_POINTER->ret_sp_val
                 : "=r"(res)
                 : "D"(num), "S"(a1_rsi)
                 : "cc", "rdx", "rcx", "r8",
                   "r9", "r10", "r11", "r15", "memory");
    return res;
}

uint64_t readchar() {
    return do_syscall(SYSCALL_READCHAR_ID, 0);
}

uint64_t putchar(char c) {
    return do_syscall(SYSCALL_PUTCHAR_ID, c);
}

uint64_t sleep(uint64_t micros) {
    return do_syscall(SYSCALL_SLEEP_ID, micros);
}