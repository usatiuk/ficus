//
// Created by Stepan Usatiuk on 25.08.2023.
//

#include "tty.hpp"

#include "LockGuard.hpp"
#include "Vector.hpp"
#include "asserts.hpp"
#include "kmem.hpp"
#include "mutex.hpp"

static Mutex ttysMutex;

Vector<tty> ttys;

void add_tty(tty_funcs funcs) {
    LockGuard l(ttysMutex);

    ttys.emplace_back(ttys.size(), funcs);
}

void tty_putchar(struct tty *tty, char c) {
    LockGuard l(tty->lock);
    tty->funcs.putchar(c);
}

void tty_putstr(struct tty *tty, const char *str) {
    LockGuard l(tty->lock);
    while (*str != '\0') tty->funcs.putchar(*str++);
}

void all_tty_putchar(char c) {
    for (unsigned i = 0; i < get_num_ttys(); i++) { tty_putchar(get_tty(i), c); }
}

void all_tty_putstr(const char *str) {
    for (unsigned i = 0; i < get_num_ttys(); i++) { tty_putstr(get_tty(i), str); }
}

unsigned get_num_ttys() {
    return ttys.size();
}

struct tty *get_tty(unsigned n) {
    if (n < get_num_ttys()) return &ttys[n];
    else
        return NULL;
}