//
// Created by Stepan Usatiuk on 26.10.2023.
//

#include "syscalls.hpp"
#include "syscalls_defs.h"

#include <cstdint>

#include "TtyManager.hpp"
#include "asserts.hpp"
#include "gdt.hpp"
#include "misc.hpp"

// Don't forget the correct order
// Shockingly, it doesn't immediately break and even something simple as putchar works
// even with completely broken 16-bit segments somehow
// But what happens with something more complex is completely bonkers
struct STAR {
    unsigned unused : 32;
    unsigned call_cs_ss : 16;
    unsigned ret_cs_ss : 16;
} __attribute__((packed));

static_assert(sizeof(STAR) == 8);

void setup_syscalls() {
    union {
        STAR star;
        uint64_t bytes;
    } newstar{};

    newstar.star.ret_cs_ss = (GDTSEL(gdt_data_user) - 8) | 0x3;
    assert(newstar.star.ret_cs_ss + 8 == (GDTSEL(gdt_data_user) | 0x3));
    assert(newstar.star.ret_cs_ss + 16 == (GDTSEL(gdt_code_user) | 0x3));

    newstar.star.call_cs_ss = (GDTSEL(gdt_code));
    assert(newstar.star.call_cs_ss == GDTSEL(gdt_code));
    assert(newstar.star.call_cs_ss + 8 == GDTSEL(gdt_data));

    wrmsr(0xc0000081, newstar.bytes);
    wrmsr(0xc0000082, reinterpret_cast<uint64_t>(&_syscall_entrypoint));
    wrmsr(0xc0000084, 0);

    wrmsr(0xC0000080, rdmsr(0xC0000080) | 0b1);
}

uint64_t syscall_putchar(char c) {
    GlobalTtyManager.all_tty_putchar(c);
    return 0;
}

uint64_t syscall_sleep(uint64_t micros) {
    sleep_self(micros);
    return 0;
}

extern "C" uint64_t syscall_impl(uint64_t id_rdi, uint64_t a1_rsi, uint64_t a2_rdx, uint64_t a3_rcx) {
    switch (id_rdi) {
        case SYSCALL_PUTCHAR_ID:
            return syscall_putchar(a1_rsi);
        case SYSCALL_SLEEP_ID:
            return syscall_sleep(a1_rsi);
        default:
            return -1;
    }
}
