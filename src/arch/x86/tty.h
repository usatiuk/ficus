//
// Created by Stepan Usatiuk on 25.08.2023.
//

#ifndef OS1_TTY_H
#define OS1_TTY_H

#include "mutex.h"
#include <stdint.h>

struct tty_funcs {
    void (*putchar)(char);
};

struct tty {
    unsigned id;
    struct Mutex lock;
    struct tty_funcs funcs;
};

unsigned add_tty(struct tty_funcs);
void tty_putchar(struct tty *tty, char c);
void tty_putstr(struct tty *tty, const char *str);

void all_tty_putchar(char c);
void all_tty_putstr(const char *str);

unsigned get_num_ttys();
struct tty *get_tty(unsigned n);

#endif//OS1_TTY_H
