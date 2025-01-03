//
// Created by Stepan Usatiuk on 26.11.2023.
//

#ifndef FICUS_TTY_HPP
#define FICUS_TTY_HPP


#include "mutex.hpp"
class Tty {
public:
    virtual void putchar(char c)         = 0;
    virtual void putstr(const char *str) = 0;
    virtual char readchar()              = 0;
};


#endif //FICUS_TTY_HPP
