#ifndef OS1_KMEM_H
#define OS1_KMEM_H

#include <stddef.h>
#include <stdint.h>

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

uint64_t get_heap_allocated();
uint64_t get_heap_used();

#endif