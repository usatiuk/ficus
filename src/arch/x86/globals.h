//
// Created by Stepan Usatiuk on 13.08.2023.
//

#ifndef OS1_GLOBALS_H
#define OS1_GLOBALS_H

#include <stdint.h>

#define KERN_STACK_SIZE (1024 * 1024)
extern uint64_t KERN_stack[KERN_STACK_SIZE] __attribute__((aligned(16)));

#endif//OS1_GLOBALS_H
