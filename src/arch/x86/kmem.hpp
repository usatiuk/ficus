#ifndef FICUS_KMEM_H
#define FICUS_KMEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#define KERN_HeapVirtBegin  (0xffffc00000000000ULL)
#define KERN_HeapMagicFree  0xDEDEDEDEDEDEDEDE
#define KERN_HeapMagicTaken 0xADADADADADADADAD

void init_kern_heap();

struct HeapEntry {
    uint64_t          magic;
    struct HeapEntry *next;
    struct HeapEntry *prev;
    uint64_t          len;
    char              data[] __attribute__((aligned(16)));
} __attribute__((packed, aligned(16)));

extern struct HeapEntry *KERN_HeapBegin;
extern uintptr_t         KERN_HeapEnd; // Past the end

void *kmalloc(size_t n);
void  kfree(void *addr);
void *krealloc(void *addr, size_t newsize);

uint64_t get_heap_allocated();
uint64_t get_heap_used();

#ifdef __cplusplus
}
#endif

#endif