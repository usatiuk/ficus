//
// Created by Stepan Usatiuk on 23.03.2024.
//

#ifndef OS2_STDLIB_H
#define OS2_STDLIB_H

#include "kmem.hpp"

#ifdef __cplusplus
extern "C" {
#endif

static inline void  free(void *ptr) { return kfree(ptr); }
static inline void *calloc(size_t nmemb, size_t size) {
    void *ret = kmalloc(nmemb * size);
    __builtin_memset(ret, 0, nmemb * size);
    return ret;
}
static inline void *malloc(size_t size) { return kmalloc(size); }
static inline void *realloc(void *ptr, size_t size) { return krealloc(ptr, size); }

#ifdef __cplusplus
}
#endif

#endif //OS2_STDLIB_H
