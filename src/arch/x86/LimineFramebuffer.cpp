//
// Created by Stepan Usatiuk on 14.04.2024.
//

#include "LimineFramebuffer.hpp"


LimineFramebuffer::LimineFramebuffer(limine_framebuffer *limine_framebuffer) : _backing(limine_framebuffer) {
    _dimensions = {.x = limine_framebuffer->width, .y = limine_framebuffer->height};
}
void LimineFramebuffer::set(size_t x, size_t y, uint32_t color) {
    *coord_to_ptr(x, y) = color;
}
int LimineFramebuffer::get(size_t x, size_t y) {
    return *coord_to_ptr(x, y);
}
uint32_t *LimineFramebuffer::coord_to_ptr(size_t x, size_t y) {
    return &((uint32_t *) _backing->address)[x * (_backing->pitch / 4) + y];
}
