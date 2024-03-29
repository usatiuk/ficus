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

uint64_t sputchar(char c);
uint64_t sreadchar();

void     print_mem();
void     print_tasks();

#ifdef __cplusplus
}
#endif

#endif //OS2_SYSCALLS_INTERFACE_H
