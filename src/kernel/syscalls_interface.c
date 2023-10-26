//
// Created by Stepan Usatiuk on 26.10.2023.
//

#include "syscalls_interface.h"
#include "syscalls_defs.h"

uint64_t putchar(char c) {
    uint64_t res;
    uint64_t id = SYSCALL_PUTCHAR_ID;
    asm("syscall"
        : "=r"(res)
        : "Di"(id), "Si"(c)
        : "memory");
    return res;
}