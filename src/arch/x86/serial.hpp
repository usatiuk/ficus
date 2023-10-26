//
// Created by Stepan Usatiuk on 12.08.2023.
//

#ifndef OS1_SERIAL_H
#define OS1_SERIAL_H

#include "misc.hpp"

int init_serial();

int serial_received();
char read_serial();

int is_transmit_empty();
void write_serial(char a);
void writestr(const char *a);

#endif//OS1_SERIAL_H
