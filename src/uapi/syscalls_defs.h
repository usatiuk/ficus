//
// Created by Stepan Usatiuk on 26.10.2023.
//

#ifndef OS2_SYSCALLS_DEFS_H
#define OS2_SYSCALLS_DEFS_H

#ifdef __cplusplus
#include <cstdint>
extern "C" {
#else
#include <stdint.h>
#endif

#define SYSCALL_PUTCHAR_ID 1
#define SYSCALL_SLEEP_ID 2
#define SYSCALL_READCHAR_ID 3

#ifdef __cplusplus
}
#endif

#endif//OS2_SYSCALLS_DEFS_H
