//
// Created by Stepan Usatiuk on 12.08.2023.
//

#ifndef FICUS_LIMINE_MM_H
#define FICUS_LIMINE_MM_H

#include "limine.h"

#define LIMINE_MM_MAX 256

extern unsigned int               limine_mm_count;
extern struct limine_memmap_entry limine_mm_entries[LIMINE_MM_MAX];
extern unsigned int               limine_mm_overflow;

void limine_mm_save_response();

#endif //FICUS_LIMINE_MM_H
