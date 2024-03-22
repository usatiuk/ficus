//
// Created by Stepan Usatiuk on 26.10.2023.
//

#ifndef OS2_SYSCALLS_HPP
#define OS2_SYSCALLS_HPP

#include <cstdint>

void                setup_syscalls();

extern "C" void     _syscall_entrypoint();
extern "C" uint64_t syscall_impl(uint64_t id_rdi, uint64_t a1_rsi, uint64_t a2_rdx, uint64_t a3_rcx);

#endif //OS2_SYSCALLS_HPP
