//
// Created by Stepan Usatiuk on 26.10.2023.
//

#ifndef OS2_SYSCALLS_INTERFACE_H
#define OS2_SYSCALLS_INTERFACE_H

#ifdef __cplusplus
#include <cstdint>
extern "C" {
#else
#include <stdint.h>
#endif

uint64_t putchar(char c);
uint64_t readchar();
uint64_t sleep(uint64_t micros);

#ifdef __cplusplus
}
#endif

#endif//OS2_SYSCALLS_INTERFACE_H
