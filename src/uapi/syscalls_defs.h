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

#include "FileOpts.h"

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


#define SYSCALL_PRINT_MEM   1000
#define SYSCALL_PRINT_TASKS 1001

#ifdef __cplusplus
}
#endif

#endif //OS2_SYSCALLS_DEFS_H
