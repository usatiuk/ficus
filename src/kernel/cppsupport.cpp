//
// Created by Stepan Usatiuk on 21.10.2023.
//

#include <cstddef>
#include <cstdint>

#include "kmem.hpp"
#include "misc.hpp"
#include "serial.hpp"

#if UINT32_MAX == UINTPTR_MAX
#define STACK_CHK_GUARD 0xb079a218
#else
#define STACK_CHK_GUARD 0x2e61e13e4d5ae23c
#endif

uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

extern "C" __attribute__((noreturn)) void __stack_chk_fail(void) {
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

    extern "C" int __cxa_guard_acquire(__guard *);
    extern "C" void __cxa_guard_release(__guard *);
    extern "C" void __cxa_guard_abort(__guard *);

    extern "C" int __cxa_guard_acquire(__guard *g) {
        return !*(char *) (g);
    }

    extern "C" void __cxa_guard_release(__guard *g) {
        *(char *) g = 1;
    }

    extern "C" void __cxa_guard_abort(__guard *) {
        _hcf();
    }
}// namespace __cxxabiv1

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