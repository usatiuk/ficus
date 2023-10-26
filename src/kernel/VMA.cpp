//
// Created by Stepan Usatiuk on 26.10.2023.
//

#include "VMA.hpp"

#include "asserts.hpp"
#include "kmem.hpp"
#include "memman.hpp"
#include "paging.hpp"

VMA::VMA(AddressSpace *space) : space(space) {
}

void VMA::mark_taken(void *addr, size_t length) {
}

void VMA::map_kern() {
    for (uintptr_t i = (uint64_t) (0xFFF8000000000000ULL >> 39) & 0x01FF; i < 512; i++) {
        space->get_cr3()[i] = KERN_AddressSpace->get_cr3()[i];
    }
}

void *VMA::mmap_phys(void *v_addr, void *real_addr, size_t length, int flags) {
    assert((((uintptr_t) v_addr) & PAGE_SIZE) == 0);

    for (size_t i = 0; i < length; i += PAGE_SIZE) {
        space->map(v_addr + i, real_addr + i, flags);
    }
    return v_addr;
}
void *VMA::mmap_mem_any(size_t length, int prot, int flags) {
    return nullptr;
}
int VMA::munmap(void *addr, size_t length) {
    return 0;
}
