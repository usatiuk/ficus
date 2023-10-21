//
// Created by Stepan Usatiuk on 09.08.2023.
//

#ifndef OS1_PAGING_H
#define OS1_PAGING_H

#include <stddef.h>
#include <stdint.h>

struct AddressSpace {
    // Pointer to PML4 in HDDM
    uint64_t *PML4;
};

extern struct AddressSpace *KERN_AddressSpace;

int init_addr_space(struct AddressSpace *space);

extern uintptr_t kernel_phys_base;
extern uintptr_t kernel_virt_base;
void limine_kern_save_response();

#define KERN_V2P(a) ((((uintptr_t) (a) + kernel_phys_base) & ~kernel_virt_base))
#define KERN_P2V(a) ((((uintptr_t) (a) -kernel_phys_base) | kernel_virt_base))

#define HHDM_BEGIN 0xfffff80000000000ULL
#define HHDM_SIZE 32ULL// In GB
#define HHDM_V2P(a) ((((uintptr_t) (a)) & ~HHDM_BEGIN))
#define HHDM_P2V(a) ((((uintptr_t) (a)) | HHDM_BEGIN))

#define PAGE_PS (1 << 7)
#define PAGE_RW (1 << 1)
#define PAGE_USER (1 << 2)
#define PAGE_PRESENT (0x01ULL)

int map(void *virt, void *real, uint32_t flags, struct AddressSpace *space);
int unmap(void *virt, struct AddressSpace *space);
void *virt2real(void *virt, struct AddressSpace *space);

void map_hddm(uint64_t *pml4);

extern "C" void _tlb_flush();

#endif//OS1_PAGING_H