//
// Created by Stepan Usatiuk on 21.10.2023.
//

#ifndef FICUS_STRING_H
#define FICUS_STRING_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

inline void *memcpy(void *dest, const void *src, size_t n) {
    uint8_t       *pdest = (uint8_t *) dest;
    const uint8_t *psrc  = (const uint8_t *) src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

inline void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *) s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t) c;
    }

    return s;
}

inline void *memmove(void *dest, const void *src, size_t n) {
    uint8_t       *pdest = (uint8_t *) dest;
    const uint8_t *psrc  = (const uint8_t *) src;

    if (src > dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i - 1] = psrc[i - 1];
        }
    }

    return dest;
}

inline int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = (const uint8_t *) s1;
    const uint8_t *p2 = (const uint8_t *) s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

inline uint64_t strlen(const char *str) {
    uint64_t res = 0;
    while (*(str++) != '\0') res++;
    return res;
}

inline char *strcat(char *dst, const char *src) {
    char *dstw = dst;

    while (*(dstw) != '\0')
        dstw++;

    int i = 0;
    while (src[i] != '\0') {
        dstw[i] = src[i];
        i++;
    }
    dstw[i] = '\0';

    return dst;
}

inline int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *((const unsigned char *) s1) - *((const unsigned char *) s2);
}

inline void strcpy(const char *src, char *dst) {
    int i = 0;
    while (src[i] != '\0') {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

inline void strncpy(char *dst, const char *src, size_t bufsize) {
    int i = 0;
    while (src[i] != '\0' && (i < (bufsize - 1))) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}


#ifdef __cplusplus
}
#endif

#endif //FICUS_STRING_H
