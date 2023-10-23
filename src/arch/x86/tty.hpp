//
// Created by Stepan Usatiuk on 25.08.2023.
//

#ifndef OS1_TTY_H
#define OS1_TTY_H

#include "mutex.hpp"

struct tty_funcs {
    void (*putchar)(char);
};

struct tty {
    Mutex lock;
    unsigned id;
    tty_funcs funcs;

    tty(unsigned id, tty_funcs funcs) : id(id), funcs(funcs) {}
};

void add_tty(struct tty_funcs);
void tty_putchar(struct tty *tty, char c);
void tty_putstr(struct tty *tty, const char *str);

void all_tty_putchar(char c);
void all_tty_putstr(const char *str);

unsigned get_num_ttys();
struct tty *get_tty(unsigned n);

#endif//OS1_TTY_H
