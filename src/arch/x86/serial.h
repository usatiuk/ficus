//
// Created by Stepan Usatiuk on 12.08.2023.
//

#ifndef OS1_SERIAL_H
#define OS1_SERIAL_H

#include "misc.h"

int init_serial();

int serial_received();
char read_serial();

int is_transmit_empty();
void write_serial(char a);
void writestr(const char *a);

static inline void _assert2(int val, const char *msg) {
    if (!val) {
        writestr(msg);
        _hcf();
    }
}

#define assert2(x, y) _assert2(x, y)
#define assert(x) _assert2(x, "Assertion failed")

#endif//OS1_SERIAL_H
