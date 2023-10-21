//
// Created by Stepan Usatiuk on 12.08.2023.
//

#ifndef OS1_MEMMAN_H
#define OS1_MEMMAN_H

#include "limine.h"
enum PageStatus {
    MEMMAN_STATE_FREE = 1,
    MEMMAN_STATE_USED = 2,
    MEMMAN_STATE_RESERVED = 0,
    MEMMAN_STATE_RECLAIMABLE = 3,
};

struct FourPages {
    enum PageStatus first : 2;
    enum PageStatus second : 2;
    enum PageStatus third : 2;
    enum PageStatus fourth : 2;
};

void parse_limine_memmap(struct limine_memmap_entry *entries, unsigned int num, uint64_t what_is_considered_free);

void *get4k();
void free4k(void *page);
uint64_t get_free();

#endif//OS1_MEMMAN_H
