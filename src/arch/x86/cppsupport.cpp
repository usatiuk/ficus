//
// Created by Stepan Usatiuk on 21.10.2023.
//

#include <cstddef>

#include "kmem.hpp"
#include "misc.hpp"

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