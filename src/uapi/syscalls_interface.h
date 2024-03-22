//
// Created by Stepan Usatiuk on 26.10.2023.
//

#ifndef OS2_SYSCALLS_INTERFACE_H
#define OS2_SYSCALLS_INTERFACE_H

#include "dirent.h"

#ifdef __cplusplus
#include <cstdint>
extern "C" {
#else
#include <stdint.h>
#endif

#include "FileOpts.h"

uint64_t putchar(char c);
uint64_t readchar();
uint64_t sleep(uint64_t micros);

uint64_t open(const char *pathname, int flags);
uint64_t close(uint64_t FD);

uint64_t read(uint64_t fd, char *buf, uint64_t len);
uint64_t write(uint64_t fd, const char *buf, uint64_t len);
uint64_t lseek(uint64_t fd, uint64_t off, uint64_t whence);

#ifdef __cplusplus
}
#endif

#endif//OS2_SYSCALLS_INTERFACE_H
