//
// Created by Stepan Usatiuk on 26.10.2023.
//

#ifndef FICUS_VMA_HPP
#define FICUS_VMA_HPP

#include "SkipList.hpp"
#include "Spinlock.hpp"
#include "mutex.hpp"
#include <cstddef>
#include <cstdint>
#include <optional>

class AddressSpace;

class VMA {
public:
     VMA(AddressSpace *space);
    ~VMA();

         VMA(const VMA &)       = delete;
         VMA(VMA &&)            = delete;
    VMA &operator=(const VMA &) = delete;
    VMA &operator=(VMA &&)      = delete;


    /// Map all higher-half pages into the address space
    /// By linking them to same entries as kernel
    void  map_kern();
    void *mmap_phys(void *v_addr, void *real_addr, size_t length, int flags);
    void *mmap_mem(void *v_addr, size_t length, int prot, int flags);
    int   munmap(void *addr, size_t length);

    static constexpr size_t kBrkSize =   4096ULL * 20ULL;
    std::optional<char *>   brk_start;
    std::optional<char *>   brk_end_fake;
    std::optional<char *>   brk_end_real;

    void clone_from(const VMA &vma);

private:
    AddressSpace *space = nullptr;
    Mutex         space_lock;

    enum class EntryType {
        FREE,
        PHYS,
        ANON
    };

    struct ListEntry {
        uintptr_t begin;
        uint64_t  length;
        EntryType type = EntryType::FREE;
        int       flags;
    };

    ListEntry *get_entry(uintptr_t v_addr, size_t length);

    //
    SkipListMap<uintptr_t, ListEntry> regions;
    Mutex                             regions_lock;
};


#endif //FICUS_VMA_HPP
