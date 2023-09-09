#ifndef OS1_KMEM_H
#define OS1_KMEM_H

#include <stddef.h>
#include <stdint.h>

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
uint64_t strlen(char *str);
void strcpy(const char *src, char *dst);

#define KERN_HeapVirtBegin (0xffffc00000000000ULL)
#define KERN_HeapMagicFree 0xDEDE
#define KERN_HeapMagicTaken 0xADAD

void init_kern_heap();

struct HeapEntry {
    uint_fast16_t magic;
    struct HeapEntry *next;
    struct HeapEntry *prev;
    uint64_t len;
    char data[];
};

extern struct HeapEntry *KERN_HeapBegin;
extern uintptr_t KERN_HeapEnd;// Past the end

void *kmalloc(size_t n);
void kfree(void *addr);
void *krealloc(void *addr, size_t newsize);

#endif