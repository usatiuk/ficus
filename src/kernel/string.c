//
// Created by Stepan Usatiuk on 22.03.2024.
//

#include <stddef.h>
#include <stdint.h>

// GCC Bug? even with freestanding and no-builtin gcc tries calling that
void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *) s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t) c;
    }

    return s;
}