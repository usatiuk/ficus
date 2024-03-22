//
// Created by Stepan Usatiuk on 25.08.2023.
//

#include "TtyManager.hpp"

#include "LockGuard.hpp"
#include "Vector.hpp"
#include "mutex.hpp"

TtyManager         GlobalTtyManager;

Vector<TtyManager> ttys;

void               TtyManager::add_tty(Tty *tty) {
    LockGuard l(lock);
    ttys.emplace_back(tty);
}

void TtyManager::all_tty_putchar(char c) {
    for (unsigned i = 0; i < get_num_ttys(); i++) { get_tty(i)->putchar(c); }
}

void TtyManager::all_tty_putstr(const char *str) {
    for (unsigned i = 0; i < get_num_ttys(); i++) { get_tty(i)->putstr(str); }
}

unsigned TtyManager::get_num_ttys() {
    return ttys.size();
}

Tty *TtyManager::get_tty(unsigned n) {
    if (n < get_num_ttys()) return ttys[n];
    else
        return NULL;
}