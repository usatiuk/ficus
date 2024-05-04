//
// Created by Stepan Usatiuk on 14.04.2024.
//

#ifndef FRAMEBUFFER_HPP
#define FRAMEBUFFER_HPP


#include <cstddef>
#include <cstdint>
#include <utility>


class Framebuffer {
public:
    struct dimensions_t {
        size_t x;
        size_t y;
    };

    dimensions_t dimensions() { return _dimensions; }

    virtual void set(size_t x, size_t y, uint32_t color) = 0;
    virtual int  get(size_t x, size_t y)                 = 0;

protected:
    dimensions_t _dimensions;
};


#endif //FRAMEBUFFER_HPP
