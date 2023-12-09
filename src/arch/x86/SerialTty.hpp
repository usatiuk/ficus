//
// Created by Stepan Usatiuk on 26.11.2023.
//

#ifndef OS2_SERIALTTY_HPP
#define OS2_SERIALTTY_HPP

#include "CircularBuffer.hpp"
#include "Tty.hpp"
#include "cv.hpp"

class SerialTty : public Tty {
    // TODO: Possibly there should be 2 mutexes?
    Mutex mutex;
    CV readercv;
    CV isrcv;
    static void isr(void *tty);

    void this_isr();
    void this_pooler();
    CircularBuffer<char, 512> buf;

public:
    SerialTty();
    void putchar(char c) override;
    void putstr(const char *str) override;
    char readchar() override;
};


#endif//OS2_SERIALTTY_HPP
