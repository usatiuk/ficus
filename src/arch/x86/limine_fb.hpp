//
// Created by Stepan Usatiuk on 12.08.2023.
//

#ifndef FICUS_LIMINE_FB_H
#define FICUS_LIMINE_FB_H

#include "limine.h"

#include "paging.hpp"

void limine_fb_save_response(struct AddressSpace *boot_address_space);
void limine_fb_remap(struct AddressSpace *space);

extern int                       framebuffer_count;
extern struct limine_framebuffer framebuffers[10];


#endif //FICUS_LIMINE_FB_H
