//
// Created by Stepan Usatiuk on 25.08.2023.
//

#include "tty.hpp"

#include "kmem.hpp"
#include "mutex.hpp"
#include "serial.hpp"

static unsigned ttyNum = 0;
static struct Mutex ttysMutex;

struct ttys {
    unsigned num;
    struct tty *ttys;
};

struct ttys ttys = {.num = 0};

void add_tty(struct tty_funcs funcs) {
    m_lock(&ttysMutex);

    if (ttyNum >= ttys.num) {
        if (ttys.num == 0) {
            ttys.ttys = static_cast<tty *>(kmalloc(sizeof(struct ttys) + sizeof(struct tty)));
            ttys.num = 1;
        } else {
            ttys.num *= 2;
            ttys.ttys = static_cast<tty *>(krealloc(ttys.ttys, sizeof(struct ttys) + sizeof(struct tty) * ttys.num));
        }
        assert2(ttys.ttys != NULL, "Couldn't allocate memory for ttys!");
    }

    m_init(&ttys.ttys[ttyNum].lock);
    ttys.ttys[ttyNum].id = ttyNum;
    ttys.ttys[ttyNum].funcs = funcs;

    ttyNum++;

    m_unlock(&ttysMutex);
}

void tty_putchar(struct tty *tty, char c) {
    m_lock(&tty->lock);
    tty->funcs.putchar(c);
    m_unlock(&tty->lock);
}

void tty_putstr(struct tty *tty, const char *str) {
    m_lock(&tty->lock);
    while (*str != '\0') tty->funcs.putchar(*str++);
    m_unlock(&tty->lock);
}

void all_tty_putchar(char c) {
    for (unsigned i = 0; i < ttyNum; i++) { tty_putchar(get_tty(i), c); }
}

void all_tty_putstr(const char *str) {
    for (unsigned i = 0; i < ttyNum; i++) { tty_putstr(get_tty(i), str); }
}

unsigned get_num_ttys() {
    return ttyNum;
}

struct tty *get_tty(unsigned n) {
    if (n < get_num_ttys()) return &ttys.ttys[n];
    else
        return NULL;
}