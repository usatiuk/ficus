//
// Created by Stepan Usatiuk on 09.08.2023.
//

#ifndef OS1_PAGING_H
#define OS1_PAGING_H

#include <stddef.h>
#include <stdint.h>

#include "PointersCollection.hpp"

#define PAGE_SIZE   4096

#define KERN_V2P(a) ((((uintptr_t) (a) - (uintptr_t) kernel_virt_base) + (uintptr_t) kernel_phys_base))
#define KERN_P2V(a) ((((uintptr_t) (a) -kernel_phys_base) | kernel_virt_base))

#define HHDM_BEGIN  0xfffff80000000000ULL
#define HHDM_SIZE   32ULL // In GB
#define HHDM_V2P(a) ((((uintptr_t) (a)) & ~HHDM_BEGIN))
#define HHDM_P2V(a) ((((uintptr_t) (a)) | HHDM_BEGIN))

class FDT;

class AddressSpace {
public:
    AddressSpace();
    AddressSpace(uint64_t *PML4);
    ~AddressSpace();

    void     *virt2real(void *virt);
    int       map(void *virt, void *real, uint32_t flags);
    int       unmap(void *virt);

    uint64_t *get_cr3() {
        return PML4;
    }

    uint64_t *get_cr3_phys() {
        return (uint64_t *) HHDM_V2P(PML4);
    }

    FDT *getFdt();

private:
    // Pointer to PML4 in HHDM
    uint64_t *PML4;

    FDT      *_fdt = nullptr;
};

extern AddressSpace *KERN_AddressSpace;

extern uintptr_t     kernel_phys_base;
extern uintptr_t     kernel_virt_base;
void                 limine_kern_save_response();

#define PAGE_PS      (1 << 7)
#define PAGE_RW      (1 << 1)
#define PAGE_USER    (1 << 2)
#define PAGE_PRESENT (0x01ULL)

void            map_hhdm(uint64_t *pml4);

extern "C" void _tlb_flush();

#endif //OS1_PAGING_H
