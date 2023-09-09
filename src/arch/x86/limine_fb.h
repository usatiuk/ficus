//
// Created by Stepan Usatiuk on 12.08.2023.
//

#ifndef OS1_LIMINE_FB_H
#define OS1_LIMINE_FB_H

#include "limine.h"

#include "paging.h"

void limine_fb_save_response(struct AddressSpace* boot_address_space);
void limine_fb_remap(struct AddressSpace* space);

extern int framebuffer_count;
extern struct limine_framebuffer framebuffers[10];


#endif//OS1_LIMINE_FB_H
