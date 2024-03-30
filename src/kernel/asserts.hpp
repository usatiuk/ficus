//
// Created by Stepan Usatiuk on 26.10.2023.
//

#ifndef FICUS_ASSERTS_HPP
#define FICUS_ASSERTS_HPP

#include "misc.hpp"
#include "serial.hpp"

extern "C" {
static inline void _assert2(int val, const char *msg) {
    if (!val) {
        writestr_no_yield(msg);
        _hcf();
    }
}
}

#define assert2(x, y) _assert2(x, y)
#define assert(x)     _assert2(x, "Assertion failed")


#endif //FICUS_ASSERTS_HPP
