//
// Created by Stepan Usatiuk on 26.10.2023.
//

#ifndef OS2_VMA_HPP
#define OS2_VMA_HPP

#include "SkipList.hpp"
#include "Spinlock.hpp"
#include "mutex.hpp"
#include <cstddef>
#include <cstdint>

class AddressSpace;

class VMA {
public:
    VMA(AddressSpace *space);

    void mark_taken(void *addr, size_t length);

    /// Map all higher-half pages into the address space
    /// By linking them to same entries as kernel
    void  map_kern();
    void *mmap_phys(void *v_addr, void *real_addr, size_t length, int flags);
    void *mmap_mem(void *v_addr, size_t length, int prot, int flags);
    int   munmap(void *addr, size_t length);

private:
    AddressSpace *space = nullptr;
    Mutex         space_lock;

    struct ListEntry {
        uintptr_t begin;
        uint64_t  length;
        bool      available;
    };

    SkipList<uintptr_t, ListEntry> regions;
    Mutex                          regions_lock;
};


#endif //OS2_VMA_HPP
