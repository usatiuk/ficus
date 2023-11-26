//
// Created by Stepan Usatiuk on 26.11.2023.
//

#ifndef OS2_SERIALTTY_HPP
#define OS2_SERIALTTY_HPP

#include "Tty.hpp"

class SerialTty : public Tty {
    Mutex mutex;

public:
    void putchar(char c) override;
    void putstr(const char *str) override;
};


#endif//OS2_SERIALTTY_HPP
