//
// Created by Stepan Usatiuk on 21.10.2023.
//

#include <cstddef>
#include <cstdint>

#include "asserts.hpp"
#include "kmem.hpp"
#include "misc.hpp"

#if UINT32_MAX == UINTPTR_MAX
#define STACK_CHK_GUARD 0xb079a218
#else
#define STACK_CHK_GUARD 0x2e61e13e4d5ae23c
#endif

__attribute__((used)) uintptr_t                 __stack_chk_guard = STACK_CHK_GUARD;

extern "C" __attribute__((noreturn, used)) void __stack_chk_fail(void) {
    assert2(false, "Stack protection triggered!");
}

extern "C" void __cxa_pure_virtual() {
    // Do nothing or print an error message.
}

// TODO: rewrite for multiprocessing
namespace __cxxabiv1 {
    /* guard variables */

    /* The ABI requires a 64-bit type.  */
    __extension__ typedef int __guard __attribute__((mode(__DI__)));

    extern "C" int            __cxa_guard_acquire(__guard *);
    extern "C" void           __cxa_guard_release(__guard *);
    extern "C" void           __cxa_guard_abort(__guard *);

    extern "C" int            __cxa_guard_acquire(__guard *g) {
        return !*(char *) (g);
    }

    extern "C" void __cxa_guard_release(__guard *g) {
        *(char *) g = 1;
    }

    extern "C" void __cxa_guard_abort(__guard *) {
        _hcf();
    }
} // namespace __cxxabiv1

void *operator new(size_t size) {
    return kmalloc(size);
}

void *operator new[](size_t size) {
    return kmalloc(size);
}

void operator delete(void *p) {
    kfree(p);
}

void operator delete[](void *p) {
    kfree(p);
}

void operator delete(void *p, size_t n) {
    kfree(p);
}

void operator delete[](void *p, size_t n) {
    kfree(p);
}

extern "C" {

void *__dso_handle = nullptr;

// Do we really care about destructors at kernel exit?
int __cxa_atexit(void (*f)(void *), void *objptr, void *dso) {
    writestr_no_yield("Something registered\n");
    return 0;
};

struct source_location {
    const char *file;
    uint32_t    line;
    uint32_t    column;
};

struct type_descriptor {
    uint16_t kind;
    uint16_t info;
    char    *name;
};

struct type_mismatch_info {
    struct source_location  location;
    struct type_descriptor *type;
    uintptr_t               alignment;
    uint8_t                 type_check_kind;
};

#define SAN_STOP                     true

#define is_aligned(value, alignment) !(value & (alignment - 1))
__attribute__((used)) void __ubsan_handle_type_mismatch_v1(struct type_mismatch_info *type_mismatch,
                                                           uintptr_t                  pointer) {
    struct source_location *location = &type_mismatch->location;
    if (pointer == 0) {
        writestr_no_yield("Warning: null pointer access \n");
    } else if (type_mismatch->alignment != 0 &&
               is_aligned(pointer, type_mismatch->alignment)) {
        // Most useful on architectures with stricter memory alignment requirements, like ARM.
        writestr_no_yield("Warning: unaligned memory access \n");
    } else {
        writestr_no_yield("Warning: insufficient size \n");
    }
    //    log_location(location);
    //    writestr_no_yield(" \n");
    if (SAN_STOP) _hcf();
}

__attribute__((used)) void __ubsan_handle_pointer_overflow() {
    writestr_no_yield("Warning: pointer overflow\n");
    if (SAN_STOP) _hcf();
}
__attribute__((used)) void __ubsan_handle_load_invalid_value() {
    writestr_no_yield("Warning: invalid value load\n");
    if (SAN_STOP) _hcf();
}
__attribute__((used)) void __ubsan_handle_out_of_bounds() {
    writestr_no_yield("Warning: out of bounds\n");
    if (SAN_STOP) _hcf();
}
__attribute__((used)) void __ubsan_handle_add_overflow() {
    writestr_no_yield("Warning: add overflow\n");
    if (SAN_STOP) _hcf();
}
__attribute__((used)) void __ubsan_handle_missing_return() {
    writestr_no_yield("Warning: missing return\n");
    if (SAN_STOP) _hcf();
}
__attribute__((used)) void __ubsan_handle_sub_overflow() {
    writestr_no_yield("Warning: sub overflow\n");
    if (SAN_STOP) _hcf();
}
__attribute__((used)) void __ubsan_handle_shift_out_of_bounds() {
    writestr_no_yield("Warning: shift overflow\n");
    if (SAN_STOP) _hcf();
}
__attribute__((used)) void __ubsan_handle_builtin_unreachable() {
    writestr_no_yield("Warning: unreachable\n");
    if (SAN_STOP) _hcf();
}
__attribute__((used)) void __ubsan_handle_mul_overflow() {
    writestr_no_yield("Warning: multiplication overflow\n");
    if (SAN_STOP) _hcf();
}
__attribute__((used)) void __ubsan_handle_divrem_overflow() {
    writestr_no_yield("Warning: division overflow\n");
    if (SAN_STOP) _hcf();
}
__attribute__((used)) void __ubsan_handle_nonnull_arg() {
    writestr_no_yield("Warning: null argument\n");
    if (SAN_STOP) _hcf();
}
};
