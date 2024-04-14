//
// Created by Stepan Usatiuk on 26.10.2023.
//

#include "VMA.hpp"

#include <optional>

#include "LockGuard.hpp"
#include "assert.h"
#include "kmem.hpp"
#include "memman.hpp"
#include "paging.hpp"

VMA::VMA(AddressSpace *space) : space(space) {
    LockGuard l(regions_lock);
    regions.emplace(std::make_pair<uintptr_t, ListEntry>(0x1000, {0x1000, 0xFFF8000000000000ULL - 0x20000, EntryType::FREE, PAGE_RW}));
}

VMA::ListEntry *VMA::get_entry(uintptr_t v_addr, size_t length) {
    ListEntry *found = nullptr;
    {
        LockGuard l(regions_lock);

        // Find the region with start before or at v_addr
        if (v_addr) {
            auto ub = regions.upper_bound(v_addr);
            --ub;
            found = &ub->second;
            // Check if it fits
            if (found->length < length || found->type != EntryType::FREE) found = nullptr;
        }

        // Otherwise try to find something else
        if (!found)
            for (auto &n: regions) {
                if (n.second.type == EntryType::FREE && n.second.length >= length) {
                    found = &n.second;
                    break;
                }
            }

        if (!found)
            return nullptr;

        ListEntry tmpFound = *found;
        regions.erase(found->begin);

        // If our region actually starts before what we requested, then cut it up
        if ((tmpFound.begin < v_addr) && (tmpFound.length > ((v_addr - tmpFound.begin) + length))) {
            regions.emplace(std::make_pair<uintptr_t, ListEntry>((uintptr_t) tmpFound.begin, {tmpFound.begin, v_addr - tmpFound.begin, EntryType::FREE}));
            tmpFound.begin = v_addr;
            tmpFound.length -= v_addr - tmpFound.begin;
        }

        regions.emplace(std::make_pair<uintptr_t, ListEntry>(tmpFound.begin + length, {tmpFound.begin + length, tmpFound.length - length, EntryType::FREE}));
        found = &regions.emplace(std::make_pair<uintptr_t, ListEntry>((uintptr_t) tmpFound.begin, {tmpFound.begin, length, EntryType::ANON})).first->second;
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
    entry->type  = EntryType::PHYS;
    entry->flags = flags;

    for (size_t i = 0; i < length; i += PAGE_SIZE) {
        space->map((char *) v_addr + i, (char *) real_addr + i, flags);
    }
    return v_addr;
}

void *VMA::mmap_mem(void *v_addr, size_t length, int prot, int flags) {
    assert((length & (PAGE_SIZE - 1)) == 0);

    uint64_t page_len = length / PAGE_SIZE;

    //
    ListEntry *found = get_entry(reinterpret_cast<uintptr_t>(v_addr), length);
    if (!found) return nullptr;
    found->flags = flags;
    found->type  = EntryType::ANON;

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
    assert((length & (PAGE_SIZE - 1)) == 0);
    assert((((uintptr_t) addr) & (PAGE_SIZE - 1)) == 0);

    uint64_t page_len = length / PAGE_SIZE;

    uintptr_t end = (uintptr_t) addr + length;
    uintptr_t cur = (uintptr_t) addr;

    while (cur < end) {
        LockGuard l(regions_lock);
        auto      found = regions.upper_bound(cur);
        --found;

        if (found->second.type == EntryType::FREE) {
            cur += found->second.length;
            continue;
        }

        ListEntry old = found->second;
        regions.erase(found);

        if (old.begin < cur)
            regions.emplace(std::make_pair<uintptr_t, ListEntry>((uintptr_t) old.begin, {old.begin, cur - old.begin, old.type, old.flags}));

        old.length = old.length - (cur - old.begin);
        old.begin  = cur;

        if (old.begin + old.length > end)
            regions.emplace(std::make_pair<uintptr_t, ListEntry>((uintptr_t) end, {end, (old.begin + old.length) - end, old.type, old.flags}));

        old.length = std::min(old.length, length);

        size_t new_free_len = old.length;


        uint64_t cur_page_len = new_free_len / PAGE_SIZE;

        for (int i = 0; i < cur_page_len; i++) {
            free4k((void *) HHDM_P2V(space->virt2real(reinterpret_cast<void *>(old.begin + i * PAGE_SIZE))));
            {
                LockGuard l(space_lock);
                space->unmap(reinterpret_cast<void *>(old.begin + i * PAGE_SIZE));
            }
        }

        // Merge free pages together
        auto ub = regions.upper_bound(cur);
        while (ub != regions.end() && ub->second.begin == (cur + new_free_len) && ub->second.type == EntryType::FREE) {
            auto ubold = ub;
            ++ub;
            new_free_len += ubold->second.length;
            regions.erase(ubold);
        }
        ub = regions.upper_bound(cur);
        --ub;
        while (ub != regions.end() && (ub->second.begin + ub->second.length) == cur && ub->second.type == EntryType::FREE) {
            auto ubold = ub;
            --ub;
            cur = ubold->second.begin;
            new_free_len += ubold->second.length;
            regions.erase(ubold);
        }

        regions.emplace(std::make_pair<uintptr_t, ListEntry>((uintptr_t) cur, {cur, new_free_len, EntryType::FREE, old.flags}));

        cur = cur + new_free_len;
    }
    return 0;
}
VMA::~VMA() {
    for (const auto &e: regions) {
        if (e.second.type == EntryType::ANON) {
            assert((e.second.length & (PAGE_SIZE - 1)) == 0);
            uint64_t page_len = e.second.length / PAGE_SIZE;
            for (int i = 0; i < page_len; i++) {
                free4k((void *) HHDM_P2V(space->virt2real(reinterpret_cast<void *>(e.second.begin + i * PAGE_SIZE))));
            }
        }
    }
}
void VMA::clone_from(const VMA &vma) {
    for (const auto &e: vma.regions) {
        if (e.second.type == EntryType::ANON) {
            assert((e.second.length & (PAGE_SIZE - 1)) == 0);
            //FIXME:
            if (auto p = regions.find(e.first); p != regions.end() && p->second.type != EntryType::FREE) {
                assert(p->second.length == e.second.length);
            } else
                assert(mmap_mem((void *) e.first, e.second.length, 0, e.second.flags) == (void *) e.first);

            uint64_t page_len = e.second.length / PAGE_SIZE;
            for (int i = 0; i < page_len; i++) {
                memcpy((char *) HHDM_P2V(space->virt2real(reinterpret_cast<void *>(e.second.begin + i * PAGE_SIZE))),
                       (char *) HHDM_P2V(vma.space->virt2real(reinterpret_cast<void *>(e.second.begin + i * PAGE_SIZE))),
                       PAGE_SIZE);
            }
        } else {
            if (e.second.type != EntryType::FREE)
                assert(false);
        }
    }
    brk_end_fake = vma.brk_end_fake;
    brk_end_real = vma.brk_end_real;
    brk_start    = vma.brk_start;
}
