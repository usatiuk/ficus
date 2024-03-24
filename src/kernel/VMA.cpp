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
    regions.add(0x1000, {0x1000, 0xFFF8000000000000ULL - 0x20000, EntryType::FREE});
}

VMA::ListEntry *VMA::get_entry(uintptr_t v_addr, size_t length) {
    ListEntry *found = nullptr;
    {
        LockGuard l(regions_lock);

        // Find the region with start before or at v_addr
        if (v_addr) {
            found = &regions.find(v_addr)->data;
            // Check if it fits
            if (found->length < length || found->type != EntryType::FREE) found = nullptr;
        }

        // Otherwise try to find something else
        if (!found)
            for (auto &n: regions) {
                if (n.data.type == EntryType::FREE && n.data.length >= length) {
                    found = &n.data;
                    break;
                }
            }

        if (!found)
            return nullptr;

        ListEntry tmpFound = *found;
        regions.erase(found->begin);

        // If our region actually starts before what we requested, then cut it up
        if ((tmpFound.begin < v_addr) && (tmpFound.length > ((v_addr - tmpFound.begin) + length))) {
            regions.add(tmpFound.begin, {tmpFound.begin, v_addr - tmpFound.begin, EntryType::FREE});
            tmpFound.begin = v_addr;
            tmpFound.length -= v_addr - tmpFound.begin;
        }

        regions.add(tmpFound.begin + length, {tmpFound.begin + length, tmpFound.length - length, EntryType::FREE});
        found = &regions.add(tmpFound.begin, {tmpFound.begin, length, EntryType::ANON})->data;
    }
    return found;
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

    ListEntry *entry = get_entry(reinterpret_cast<uintptr_t>(v_addr), length);
    if (!entry) return nullptr;
    entry->type = EntryType::PHYS;

    for (size_t i = 0; i < length; i += PAGE_SIZE) {
        space->map((char *) v_addr + i, (char *) real_addr + i, flags);
    }
    return v_addr;
}

void *VMA::mmap_mem(void *v_addr, size_t length, int prot, int flags) {
    size_t origlen = length;
    if ((length & (PAGE_SIZE - 1)) != 0) {
        length += PAGE_SIZE - 1;
        length &= ~(PAGE_SIZE - 1);
    }
    assert((length & (PAGE_SIZE - 1)) == 0);
    assert(length >= origlen);

    uint64_t page_len = length / PAGE_SIZE;

    //
    ListEntry *found = get_entry(reinterpret_cast<uintptr_t>(v_addr), length);
    if (!found) return nullptr;

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
VMA::~VMA() {
    for (const auto &e: regions) {
        if (e.data.type == EntryType::ANON) {
            assert((e.data.length & (PAGE_SIZE - 1)) == 0);
            uint64_t page_len = e.data.length / PAGE_SIZE;
            for (int i = 0; i < page_len; i++) {
                free4k((void *) HHDM_P2V(space->virt2real(reinterpret_cast<void *>(e.data.begin + i * PAGE_SIZE))));
            }
        }
    }
}
