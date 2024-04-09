//
// Created by Stepan Usatiuk on 12.08.2023.
//

#ifndef FICUS_SERIAL_H
#define FICUS_SERIAL_H

#include "misc.h"
#ifdef __cplusplus
extern "C" {
#endif
int  init_serial();

int  serial_received();
char read_serial();

int  is_transmit_empty();
void write_serial(char a);
void writestr(const char *a);
void write_serial_no_yield(char a);
void writestr_no_yield(const char *a);
#ifdef __cplusplus
}
#endif
#endif //FICUS_SERIAL_H
