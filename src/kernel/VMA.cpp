//
// Created by Stepan Usatiuk on 26.10.2023.
//

#include "VMA.hpp"

#include <optional>

#include "LockGuard.hpp"
#include "asserts.hpp"
#include "kmem.hpp"
#include "memman.hpp"
#include "paging.hpp"

VMA::VMA(AddressSpace *space) : space(space) {
    LockGuard l(regions_lock);
    regions.add(0x10000, {0x10000, 0xFFF8000000000000ULL - 0x20000, true});
}

void VMA::mark_taken(void *addr, size_t length) {
}

void VMA::map_kern() {
    LockGuard l(space_lock);
    for (uintptr_t i = 256; i < 512; i++) {
        space->get_cr3()[i] = KERN_AddressSpace->get_cr3()[i];
    }
}

void *VMA::mmap_phys(void *v_addr, void *real_addr, size_t length, int flags) {
    LockGuard l(space_lock);
    assert((((uintptr_t) v_addr) & PAGE_SIZE) == 0);

    for (size_t i = 0; i < length; i += PAGE_SIZE) {
        space->map(v_addr + i, real_addr + i, flags);
    }
    return v_addr;
}

void *VMA::mmap_mem(void *v_addr, size_t length, int prot, int flags) {
    if ((length & (PAGE_SIZE - 1)) != 0) {
        length += PAGE_SIZE - 1;
        length &= ~(PAGE_SIZE - 1);
    }
    assert((length & (PAGE_SIZE - 1)) == 0);
    uint64_t page_len = length / PAGE_SIZE;

    std::optional<ListEntry> found;
    {
        LockGuard l(regions_lock);

        if (v_addr) {
            found = regions.find((uintptr_t) v_addr)->data;
        } else {
            for (auto &n: regions) {
                if (n.data.available && n.data.length >= length) {
                    found = n.data;
                }
            }
        }
        if (!found) return nullptr;
        regions.erase(found->begin);
        regions.add(found->begin + length, {found->begin + length, found->length - length, true});
        regions.add(found->begin, {found->begin, length, false});
    }

    for (int i = 0; i < page_len; i++) {
        void *p = get4k();
        {
            LockGuard l(space_lock);
            space->map(reinterpret_cast<void *>(found->begin + i * PAGE_SIZE), (void *) HHDM_V2P(p), flags);
        }
    }
    return reinterpret_cast<void *>(found->begin);
}
int VMA::munmap(void *addr, size_t length) {
    return 0;
}
