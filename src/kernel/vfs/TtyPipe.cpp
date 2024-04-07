//
// Created by Stepan Usatiuk on 05.04.2024.
//

#include "TtyPipe.hpp"
#include "TtyManager.hpp"
int64_t TtyPipe::read(char *buf, size_t start, size_t num) {
    auto c = buf;
    while ((c - buf) < num) {
        *c = GlobalTtyManager.get_tty(0)->readchar();
        if (*c == '\r') {
            *(c++) = '\n';
            break;
        }
        c++;
    }
    return (c - buf);
}
int64_t TtyPipe::write(const char *buf, size_t start, size_t num) {
    auto c = buf;
    while (*c != '\0' && (c - buf) < num) {
        GlobalTtyManager.all_tty_putchar(*c);
        c++;
    }
    return num;
}
size_t TtyPipe::size() {
    return 0;
}
