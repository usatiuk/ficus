#include "kmem.hpp"

#include "LockGuard.hpp"
#include "Spinlock.hpp"
#include "assert.h"
#include "globals.hpp"
#include "memman.hpp"
#include "paging.hpp"
#include "task.hpp"

#include "mutex.hpp"
#include "string.h"

struct HeapEntry *KERN_HeapBegin;
uintptr_t         KERN_HeapEnd; // Past the end

static bool initialized = false;

std::atomic<uint64_t> allocated = 0;
std::atomic<uint64_t> used      = 0;

uint64_t get_heap_allocated() {
    return allocated;
}
uint64_t get_heap_used() {
    return used;
}

static Mutex kmem_lock;

void init_kern_heap() {
    KERN_HeapBegin = static_cast<HeapEntry *>(get4k());
    allocated.fetch_add(PAGE_SIZE);
    KERN_HeapBegin->magic = KERN_HeapMagicFree;
    KERN_HeapBegin->len   = PAGE_SIZE - (sizeof(struct HeapEntry));
    KERN_HeapBegin->next  = NULL;
    KERN_HeapBegin->prev  = NULL;
    KERN_AddressSpace->map((void *) KERN_HeapVirtBegin, (void *) HHDM_V2P(KERN_HeapBegin), PAGE_RW);
    KERN_HeapBegin = (struct HeapEntry *) KERN_HeapVirtBegin;
    KERN_HeapEnd   = (KERN_HeapVirtBegin + PAGE_SIZE);
    initialized    = true;
}

static void extend_heap(size_t n_pages) {
    assert(kmem_lock.test() && kmem_lock.owner() == Scheduler::cur_task());
    for (size_t i = 0; i < n_pages; i++) {
        void *p = get4k();
        assert2(p != NULL, "Kernel out of memory!");
        KERN_AddressSpace->map((void *) KERN_HeapEnd, (void *) HHDM_V2P(p), PAGE_RW);
        KERN_HeapEnd += PAGE_SIZE;
    }
    allocated.fetch_add(n_pages * PAGE_SIZE);
}

// n is required length!
struct HeapEntry *split_entry(struct HeapEntry *what, size_t n) {
    assert(kmem_lock.test() && kmem_lock.owner() == Scheduler::cur_task());
    assert2(what->len > (n + sizeof(struct HeapEntry)), "Trying to split a heap entry that's too small!");
    assert(n <= allocated);

    assert(reinterpret_cast<uintptr_t>(what) >= KERN_HeapVirtBegin &&
           reinterpret_cast<uintptr_t>(what) < KERN_HeapEnd);

    struct HeapEntry *new_entry = (struct HeapEntry *) (((void *) what) + sizeof(struct HeapEntry) + n);

    assert(reinterpret_cast<uintptr_t>(new_entry) >= KERN_HeapVirtBegin &&
           reinterpret_cast<uintptr_t>(new_entry) < KERN_HeapEnd);

    assert(what->len <= allocated);

    new_entry->magic = KERN_HeapMagicFree;
    new_entry->next  = what->next;
    new_entry->prev  = what;
    new_entry->len   = what->len - n - sizeof(struct HeapEntry);
    what->len        = n;

    if (new_entry->next)
        new_entry->next->prev = new_entry;

    what->next = new_entry;

    assert(what->len <= allocated);
    assert(new_entry->len <= allocated);

    return new_entry;
}

void *kmalloc(size_t n) {
    assert(initialized);

    if ((n & 0xFULL) != 0) {
        size_t origN = n;
        n += 15;
        n &= ~0xFULL;
        assert(n > origN);
    }
    assert((n & 0xFULL) == 0);

    struct HeapEntry *res = NULL;
    {
        LockGuard         l(kmem_lock);
        struct HeapEntry *entry = KERN_HeapBegin;
        assert2(entry->magic == KERN_HeapMagicFree, "Bad heap!");

        struct HeapEntry *prev = NULL;

        do {
            assert2(entry->magic == KERN_HeapMagicFree, "Bad heap!");

            if (prev) {
                assert(entry->prev == prev);
                assert(prev->next == entry);
                assert(entry->prev->next == entry);
            }

            if (entry->len == n) {
                res = entry;
                if (prev) {
                    prev->next = entry->next;
                    if (entry->next)
                        entry->next->prev = prev;
                } else {
                    if (entry->next) {
                        KERN_HeapBegin    = entry->next;
                        entry->next->prev = NULL;
                    } else {
                        KERN_HeapBegin = (struct HeapEntry *) KERN_HeapEnd;
                        extend_heap(1);
                        KERN_HeapBegin->next  = NULL;
                        KERN_HeapBegin->prev  = NULL;
                        KERN_HeapBegin->magic = KERN_HeapMagicFree;
                        KERN_HeapBegin->len   = PAGE_SIZE - (sizeof(struct HeapEntry));
                    }
                }
                break;
            }
            if (entry->len > n + sizeof(struct HeapEntry)) {
                res                               = entry;
                struct HeapEntry *new_split_entry = split_entry(res, n);

                if (prev) {
                    prev->next            = new_split_entry;
                    new_split_entry->prev = prev;
                } else {
                    KERN_HeapBegin        = new_split_entry;
                    new_split_entry->prev = NULL;
                }
                if (new_split_entry->prev)
                    assert(new_split_entry->prev->magic == KERN_HeapMagicFree);
                break;
            }

            prev  = entry;
            entry = entry->next;
        } while (entry);

        if (!res) {
            entry = prev;

            assert2(entry->magic == KERN_HeapMagicFree, "Expected last tried entry to be free");
            assert2(entry->next == NULL, "Expected last tried entry to be the last");

            size_t data_needed = n + (2 * sizeof(struct HeapEntry));

            size_t pages_needed = ((data_needed & 0xFFF) == 0)
                                          ? data_needed >> 12
                                          : ((data_needed & (~0xFFF)) + 0x1000) >> 12;

            struct HeapEntry *new_entry = (struct HeapEntry *) KERN_HeapEnd;
            extend_heap(pages_needed);
            new_entry->next  = NULL;
            new_entry->prev  = entry;
            new_entry->magic = KERN_HeapMagicFree;
            new_entry->len   = (pages_needed * PAGE_SIZE) - (sizeof(struct HeapEntry));
            assert2(new_entry->len >= n, "Expected allocated heap entry to fit what we wanted");
            res = new_entry;
            if (new_entry->len > n) {
                struct HeapEntry *new_split_entry = split_entry(res, n);
                entry->next                       = new_split_entry;
                new_split_entry->prev             = entry;
                if (new_split_entry->prev)
                    assert(new_split_entry->prev->magic == KERN_HeapMagicFree);
            }
        }

        if (!res) {
            return nullptr;
        }

        //        if (res->next) res->next->prev = res->prev;
        //        if (res->prev) res->prev->next = res->next;

        res->next  = NULL;
        res->prev  = NULL;
        res->magic = KERN_HeapMagicTaken;
    }
    assert((((uintptr_t) res->data) & 0xFULL) == 0);

    for (size_t i = 0; i < n; i++) res->data[i] = 0xFEU;
    used.fetch_add(n);
    return res->data;
}

static void try_merge_fwd(struct HeapEntry *entry) {
    assert(kmem_lock.test() && kmem_lock.owner() == Scheduler::cur_task());
    assert2(entry->magic == KERN_HeapMagicFree, "Bad merge!");
    assert(entry->prev == NULL);

    struct HeapEntry *nextEntry = (struct HeapEntry *) ((uint64_t) entry + ((uint64_t) sizeof(struct HeapEntry)) + entry->len);

    while ((uint64_t) nextEntry < KERN_HeapEnd && nextEntry->magic == KERN_HeapMagicFree) {
        if (nextEntry->prev) assert(nextEntry->prev->magic == KERN_HeapMagicFree);
        if (nextEntry->next) assert(nextEntry->next->magic == KERN_HeapMagicFree);

        if (nextEntry == entry->next) {
            nextEntry->next->prev = entry;
            entry->next           = nextEntry->next;
        } else {
            assert(nextEntry->prev && nextEntry->prev->magic == KERN_HeapMagicFree);

            struct HeapEntry *victimR = nextEntry->next;
            if (victimR) {
                assert(victimR->magic == KERN_HeapMagicFree);
                victimR->prev         = nextEntry->prev;
                nextEntry->prev->next = victimR;
            } else {
                nextEntry->prev->next = NULL;
            }
        }
        entry->len = entry->len + sizeof(struct HeapEntry) + nextEntry->len;
        nextEntry  = (struct HeapEntry *) ((uint64_t) entry + sizeof(struct HeapEntry) + entry->len);
    }
}

static struct HeapEntry *try_shrink_heap(struct HeapEntry *entry) {
    assert(kmem_lock.test() && kmem_lock.owner() == Scheduler::cur_task());
    assert(entry->prev == NULL);
    if ((uint64_t) entry + sizeof(struct HeapEntry) + entry->len == KERN_HeapEnd) {
        // Shrink it if it's at least three pages
        if (entry->len + sizeof(struct HeapEntry) < PAGE_SIZE * 3) {
            return entry;
        }

        struct HeapEntry *ret = NULL;

        // Check alignment, in case of non-alignment, split
        if (((uint64_t) entry & 0xFFF) != 0) {
            // How long the entry should be to pad for the next one to be aligned
            uint64_t diff = 0x1000ULL - ((uint64_t) entry & 0xFFF);

            // Should always work as we're checking if the length is at least three pages
            // But also check if it's enough...
            if (diff <= sizeof(struct HeapEntry)) diff += 0x1000ULL;

            entry     = split_entry(entry, diff - sizeof(struct HeapEntry));
            ret       = entry->prev;
            ret->next = entry->next;
            if (entry->next)
                entry->next->prev = ret;
        } else {
            ret       = entry->next;
            ret->prev = NULL;
        }
        assert(((uint64_t) entry & 0xFFF) == 0);

        KERN_HeapEnd      = (uintptr_t) entry;
        uint64_t totallen = entry->len + sizeof(struct HeapEntry);
        assert(((uint64_t) totallen & 0xFFF) == 0);
        uint64_t total_pages = totallen / PAGE_SIZE;
        for (uint64_t i = 0; i < total_pages; i++) {
            free4k((void *) HHDM_P2V(KERN_AddressSpace->virt2real((void *) (KERN_HeapEnd + PAGE_SIZE * i))));
            allocated.fetch_sub(PAGE_SIZE);
            KERN_AddressSpace->unmap((void *) (KERN_HeapEnd + PAGE_SIZE * i));
        }
        return ret;
    }

    return entry;
}

void kfree(void *addr) {
    assert(initialized);
    LockGuard l(kmem_lock);

    struct HeapEntry *freed = (struct HeapEntry *) (addr - (sizeof(struct HeapEntry)));
    used.fetch_sub(freed->len);

    struct HeapEntry *entry = KERN_HeapBegin;
    assert2(freed->magic == KERN_HeapMagicTaken, "Bad free!");
    assert2(freed->next == NULL, "Bad free!");
    assert2(freed->prev == NULL, "Bad free!");
    assert2(entry->magic == KERN_HeapMagicFree, "Bad free!");
    assert2(entry->prev == NULL, "Bad free!");

    freed->next    = entry;
    entry->prev    = freed;
    KERN_HeapBegin = freed;
    freed->magic   = KERN_HeapMagicFree;

    try_merge_fwd(freed);
    assert2(freed->prev == NULL, "Bad free!");
    KERN_HeapBegin = try_shrink_heap(freed);
    assert(KERN_HeapBegin != NULL);
    assert2(KERN_HeapBegin->prev == NULL, "Bad free!");
}

void *krealloc(void *addr, size_t newsize) {
    assert(initialized);

    if (addr == nullptr) return kmalloc(newsize);

    struct HeapEntry *info = (struct HeapEntry *) (addr - (sizeof(struct HeapEntry)));
    assert2(info->magic == KERN_HeapMagicTaken, "Bad realloc!");

    void *newt = kmalloc(newsize);

    memcpy(newt, addr, newsize > info->len ? info->len : newsize);
    kfree(addr);

    return newt;
}