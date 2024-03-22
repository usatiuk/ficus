//
// Created by Stepan Usatiuk on 25.08.2023.
//

#ifndef OS1_TTY_H
#define OS1_TTY_H

#include "Spinlock.hpp"
#include "Tty.hpp"
#include "Vector.hpp"

class TtyManager {
    Mutex         lock;
    Vector<Tty *> ttys;

public:
    void     add_tty(Tty *tty);

    void     all_tty_putchar(char c);
    void     all_tty_putstr(const char *str);

    unsigned get_num_ttys();
    Tty     *get_tty(unsigned n);
};

extern TtyManager GlobalTtyManager;

#endif //OS1_TTY_H
