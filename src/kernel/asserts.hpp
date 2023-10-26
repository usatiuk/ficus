//
// Created by Stepan Usatiuk on 26.10.2023.
//

#ifndef OS2_ASSERTS_HPP
#define OS2_ASSERTS_HPP

#include "misc.hpp"
#include "serial.hpp"

extern "C" {
static inline void _assert2(int val, const char *msg) {
    if (!val) {
        writestr(msg);
        _hcf();
    }
}
}

#define assert2(x, y) _assert2(x, y)
#define assert(x) _assert2(x, "Assertion failed")


#endif//OS2_ASSERTS_HPP
