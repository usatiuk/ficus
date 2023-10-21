//
// Created by Stepan Usatiuk on 12.08.2023.
//

#include "limine_mm.hpp"

#include "kmem.hpp"
#include "limine.h"

static volatile struct limine_memmap_request memmap_request = {
        .id = LIMINE_MEMMAP_REQUEST,
        .revision = 0};

unsigned int limine_mm_count;
struct limine_memmap_entry limine_mm_entries[LIMINE_MM_MAX];
unsigned int limine_mm_overflow;

void limine_mm_save_response() {
    limine_mm_count = memmap_request.response->entry_count;
    if (limine_mm_count > LIMINE_MM_MAX) {
        limine_mm_count = LIMINE_MM_MAX;
        limine_mm_overflow = 1;
    } else {
        limine_mm_overflow = 0;
    }
    for (int i = 0; i < limine_mm_count; i++) {
        memcpy(&limine_mm_entries[i], memmap_request.response->entries[i], sizeof(struct limine_memmap_entry));
    }
}