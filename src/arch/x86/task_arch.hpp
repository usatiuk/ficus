//
// Created by Stepan Usatiuk on 01.04.2024.
//

#ifndef FICUS_TASK_ARCH_HPP
#define FICUS_TASK_ARCH_HPP

#include <cstdint>

namespace Arch {
    static constexpr uint64_t kIDT_GUARD = 0xdeadbe3fdeadb3efULL;

    // Assuming the compiler understands that this is pushed on the stack in the correct order
    struct TaskFrame {
        uint64_t guard;
        uint64_t guard2; // To keep stack aligned after pushaq

        uint64_t r15;
        uint64_t r14;
        uint64_t r13;
        uint64_t r12;
        uint64_t r11;
        uint64_t r10;
        uint64_t r9;
        uint64_t r8;

        uint64_t rdi;
        uint64_t rsi;
        uint64_t rbp;
        uint64_t rbx;
        uint64_t rdx;
        uint64_t rcx;
        uint64_t rax;

        uint64_t ip;
        uint64_t cs;
        uint64_t flags;
        uint64_t sp;
        uint64_t ss;
    } __attribute__((packed));

} // namespace Arch

#endif //FICUS_TASK_ARCH_HPP
