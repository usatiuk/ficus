//
// Created by Stepan Usatiuk on 26.10.2023.
//

#include "syscalls.hpp"

#include <cstdint>

#include "asserts.hpp"
#include "gdt.hpp"
#include "misc.hpp"

struct STAR {
    unsigned ret_cs_ss : 16;
    unsigned call_cs_ss : 16;
    unsigned unused : 32;
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