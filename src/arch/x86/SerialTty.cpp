//
// Created by Stepan Usatiuk on 26.11.2023.
//

#include "SerialTty.hpp"
#include "LockGuard.hpp"

void SerialTty::putchar(char c) {
    LockGuard guard(mutex);
    write_serial(c);
}

void SerialTty::putstr(const char *str) {
    LockGuard guard(mutex);
    writestr(str);
}
