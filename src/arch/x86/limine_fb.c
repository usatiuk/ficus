//
// Created by Stepan Usatiuk on 12.08.2023.
//

#include "limine_fb.h"

#include <stddef.h>

#include "kmem.h"

static volatile struct limine_framebuffer_request framebuffer_request = {
        .id = LIMINE_FRAMEBUFFER_REQUEST,
        .revision = 0};

int framebuffer_count = 0;
struct limine_framebuffer framebuffers[10];
struct {
    void *base;
    uint64_t len;
} framebufferAddrs[10];

void limine_fb_save_response(struct AddressSpace* boot_address_space) {
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        framebuffer_count = 0;
        return;
    }

    framebuffer_count = framebuffer_request.response->framebuffer_count;
    if (framebuffer_count >= 10) framebuffer_count = 10;
    for (int i = 0; i < framebuffer_count; i++) {
        memcpy(&framebuffers[i], framebuffer_request.response->framebuffers[i], sizeof(struct limine_framebuffer));
        framebufferAddrs[i].base = virt2real(framebuffers[i].address, boot_address_space);
    }
}

void limine_fb_remap(struct AddressSpace* space) {
    for (int i = 0; i < framebuffer_count; i++) {
        void *base = framebuffers[i].address;
        void *realbase = framebufferAddrs[i].base;
        // TODO: Proper map
        for (int i = 0; i < 100000; i++) {
            map(base + i * 4096, realbase + i * 4096, PAGE_RW, space);
        }
    }
    _tlb_flush();
}