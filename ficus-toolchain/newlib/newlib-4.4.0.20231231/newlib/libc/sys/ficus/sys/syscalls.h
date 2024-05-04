//
// Created by Stepan Usatiuk on 13.04.2024.
//

#ifndef _SYS_SYSCALLS_H
#define _SYS_SYSCALLS_H

#ifdef __cplusplus
extern "C" {
#endif

#define SYSCALL_EXIT_ID 0

#define SYSCALL_PUTCHAR_ID  1
#define SYSCALL_SLEEP_ID    2
#define SYSCALL_READCHAR_ID 3

#define SYSCALL_OPEN_ID  4
#define SYSCALL_CLOSE_ID 5

#define SYSCALL_READ_ID  6
#define SYSCALL_WRITE_ID 7
#define SYSCALL_LSEEK_ID 8

#define SYSCALL_OPENDIR_ID  9
#define SYSCALL_READDIR_ID  10
#define SYSCALL_CLOSEDIR_ID 11
#define SYSCALL_MKDIR_ID    12
#define SYSCALL_UNLINK_ID   13

#define SYSCALL_EXECVE_ID  50
#define SYSCALL_FORK_ID    51
#define SYSCALL_WAITPID_ID 52
#define SYSCALL_SBRK_ID    100

#define SYSCALL_PRINT_MEM   1000
#define SYSCALL_PRINT_TASKS 1001

void print_mem();
void print_tasks();

#ifdef __cplusplus
}
#endif

#endif //_SYS_SYSCALLS_H
