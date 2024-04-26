//
// Created by Stepan Usatiuk on 14.04.2024.
//

#ifndef LIMINEFRAMEBUFFER_HPP
#define LIMINEFRAMEBUFFER_HPP


#include <Framebuffer.hpp>

#include <cstddef>
#include <cstdint>

#include <limine.h>

class LimineFramebuffer : public Framebuffer {
public:
             LimineFramebuffer(limine_framebuffer *limine_framebuffer);
    virtual ~LimineFramebuffer() = default;

    void set(size_t x, size_t y, uint32_t color) override;
    int  get(size_t x, size_t y) override;

private:
    uint32_t *coord_to_ptr(size_t x, size_t y);

    limine_framebuffer *_backing;
};


#endif //LIMINEFRAMEBUFFER_HPP
