//
// Created by Stepan Usatiuk on 09.08.2023.
//

#include "paging.hpp"
#include "limine.h"
#include "memman.hpp"
#include "misc.hpp"
#include "serial.hpp"

// Returns a free page frame in HHDM
static uint64_t *get_free_frame() {
    uint64_t *res = static_cast<uint64_t *>(get4k());
    assert(res != nullptr);
    for (int j = 0; j < 512; j++)
        res[j] = 0;
    return res;
}

static inline void invlpg(void *m) {
    /* Clobber memory to avoid optimizer re-ordering access before invlpg, which may cause nasty bugs. */
    asm volatile("invlpg (%0)"
                 :
                 : "b"(m)
                 : "memory");
}

AddressSpace::AddressSpace() {
    PML4 = static_cast<uint64_t *>(get4k());
}

AddressSpace::AddressSpace(uint64_t *PML4) : PML4(PML4) {}

AddressSpace::~AddressSpace() {
    // TODO:
    free4k(PML4);
}

void *AddressSpace::virt2real(void *virt) {
    assert2(((uint64_t) virt & 0xFFF) == 0, "Trying to unmap non-aligned memory!");

    // Assuming everything related to paging is HHDM
    assert2((uint64_t) PML4 >= HHDM_BEGIN, "CR3 here must be in HDDM!");
    assert2((uint64_t) PML4 < kernel_virt_base, "CR3 here must be in HDDM!");

    uint64_t pml4i = (uint64_t) virt >> 39 & 0x01FF;
    uint64_t pdpei = (uint64_t) virt >> 30 & 0x01FF;
    uint64_t pdei = (uint64_t) virt >> 21 & 0x01FF;
    uint64_t ptsi = (uint64_t) virt >> 12 & 0x01FF;

    uint64_t *pml4e = PML4 + pml4i;
    if (!((*pml4e) & PAGE_PRESENT)) return 0;

    uint64_t *pdpeb = (uint64_t *) HHDM_P2V((*pml4e & 0x000FFFFFFFFFF000ULL));
    uint64_t *pdpee = pdpeb + pdpei;
    if (!((*pdpee) & PAGE_PRESENT)) return 0;
    // Calculations here might be incorrect
    if (*pdpee & PAGE_PS) return (void *) ((*pdpee & 0x000FFFFFFFFFF000ULL) | ((uint64_t) virt & 0x00000003FFFF000ULL));

    uint64_t *pdeb = (uint64_t *) HHDM_P2V((*pdpee & 0x000FFFFFFFFFF000ULL));
    uint64_t *pdee = pdeb + pdei;
    if (!((*pdee) & PAGE_PRESENT)) return 0;
    // Calculations here might be incorrect
    if (*pdee & PAGE_PS) return (void *) ((*pdee & 0x000FFFFFFFFFF000ULL) | ((uint64_t) virt & 0x0000000001FF000ULL));

    uint64_t *ptsb = (uint64_t *) HHDM_P2V((*pdee & 0x000FFFFFFFFFF000ULL));
    uint64_t *ptse = ptsb + ptsi;
    if (!((*ptse) & PAGE_PRESENT)) return 0;

    return (void *) (*ptse & 0x000FFFFFFFFFF000ULL);
}

int AddressSpace::map(void *virt, void *real, uint32_t flags) {
    assert2(((uint64_t) virt & 0xFFF) == 0, "Trying to map non-aligned memory!");
    assert2(((uint64_t) real & 0xFFF) == 0, "Trying to map to non-aligned memory!");

    // Assuming everything related to paging is HHDM
    assert2((uint64_t) PML4 >= HHDM_BEGIN, "CR3 here must be in HDDM!");
    assert2((uint64_t) PML4 < kernel_virt_base, "CR3 here must be in HDDM!");

    uint64_t pml4i = (uint64_t) virt >> 39 & 0x01FF;
    uint64_t pdpei = (uint64_t) virt >> 30 & 0x01FF;
    uint64_t pdei = (uint64_t) virt >> 21 & 0x01FF;
    uint64_t ptsi = (uint64_t) virt >> 12 & 0x01FF;


    uint64_t *pml4e = PML4 + pml4i;

    if (!(*pml4e & PAGE_PRESENT)) {
        uint64_t *newp = get_free_frame();

        assert2(newp != NULL, "Couldn't get a page frame!");

        *pml4e |= PAGE_PRESENT | PAGE_RW | PAGE_USER;
        *pml4e |= (uint64_t) HHDM_V2P(newp) & (uint64_t) 0x000FFFFFFFFFF000ULL;
    }

    uint64_t *pdpeb = (uint64_t *) HHDM_P2V((*pml4e & 0x000FFFFFFFFFF000ULL));
    uint64_t *pdpee = &pdpeb[pdpei];
    assert2(!(*pdpee & PAGE_PS), "Encountered an unexpected large mapping!");
    if (!(*pdpee & PAGE_PRESENT)) {
        uint64_t *newp = get_free_frame();

        assert2(newp != NULL, "Couldn't get a page frame!");

        *pdpee |= PAGE_PRESENT | PAGE_RW | PAGE_USER;
        *pdpee |= (uint64_t) HHDM_V2P(newp) & (uint64_t) 0x000FFFFFFFFFF000ULL;
    }

    uint64_t *pdeb = (uint64_t *) HHDM_P2V((*pdpee & 0x000FFFFFFFFFF000ULL));
    uint64_t *pdee = &pdeb[pdei];
    assert2(!(*pdee & PAGE_PS), "Encountered an unexpected large mapping!");
    if (!(*pdee & PAGE_PRESENT)) {
        uint64_t *newp = get_free_frame();

        assert2(newp != NULL, "Couldn't get a page frame!");

        *pdee |= PAGE_PRESENT | PAGE_RW | PAGE_USER;
        *pdee |= (uint64_t) HHDM_V2P(newp) & (uint64_t) 0x000FFFFFFFFFF000ULL;
    }

    uint64_t *ptsb = (uint64_t *) HHDM_P2V((*pdee & 0x000FFFFFFFFFF000ULL));
    uint64_t *ptse = &ptsb[ptsi];
    *ptse = ((uint64_t) real & 0x000FFFFFFFFFF000ULL) | (flags & 0xFFF) | PAGE_PRESENT;
    invlpg((void *) ((uint64_t) virt & 0x000FFFFFFFFFF000ULL));
    return 1;
}
int AddressSpace::unmap(void *virt) {
    assert2(((uint64_t) virt & 0xFFF) == 0, "Trying to map non-aligned memory!");

    // Assuming everything related to paging is HHDM
    assert2((uint64_t) PML4 >= HHDM_BEGIN, "CR3 here must be in HDDM!");
    assert2((uint64_t) PML4 < kernel_virt_base, "CR3 here must be in HDDM!");

    uint64_t pml4i = (uint64_t) virt >> 39 & 0x01FF;
    uint64_t pdpei = (uint64_t) virt >> 30 & 0x01FF;
    uint64_t pdei = (uint64_t) virt >> 21 & 0x01FF;
    uint64_t ptsi = (uint64_t) virt >> 12 & 0x01FF;

    uint64_t *pml4e = PML4 + pml4i;

    assert((*pml4e & PAGE_PRESENT));

    uint64_t *pdpeb = (uint64_t *) HHDM_P2V((*pml4e & 0x000FFFFFFFFFF000ULL));
    uint64_t *pdpee = &pdpeb[pdpei];
    assert2(!(*pdpee & PAGE_PS), "Encountered an unexpected large mapping!");
    assert((*pdpee & PAGE_PRESENT));

    uint64_t *pdeb = (uint64_t *) HHDM_P2V((*pdpee & 0x000FFFFFFFFFF000ULL));
    uint64_t *pdee = &pdeb[pdei];
    assert2(!(*pdee & PAGE_PS), "Encountered an unexpected large mapping!");
    assert((*pdee & PAGE_PRESENT));

    uint64_t *ptsb = (uint64_t *) HHDM_P2V((*pdee & 0x000FFFFFFFFFF000ULL));
    uint64_t *ptse = &ptsb[ptsi];
    assert(*ptse & PAGE_PRESENT);
    *ptse = (*ptse) & (~PAGE_PRESENT);
    invlpg((void *) ((uint64_t) virt & 0x000FFFFFFFFFF000ULL));
    return 1;
}

static volatile struct limine_kernel_address_request kernel_address_request = {
        .id = LIMINE_KERNEL_ADDRESS_REQUEST,
        .revision = 0};

void limine_kern_save_response() {
    kernel_phys_base = kernel_address_request.response->physical_base;
    kernel_virt_base = kernel_address_request.response->virtual_base;
}

#define EARLY_PAGES_SIZE ((HHDM_SIZE + 1) * 2)
static uint64_t early_pages[EARLY_PAGES_SIZE][512] __attribute__((aligned(4096)));
static uint64_t early_pages_used = 0;

uintptr_t kernel_phys_base;
uintptr_t kernel_virt_base;

void map_hddm(uint64_t *pml4) {
    assert2(kernel_virt_base != 0, "Kernel virt address not loaded!");
    assert2(kernel_phys_base != 0, "Kernel phys address not loaded!");

    // Assuming here that everything related to paging is identity mapped
    // Which is true if the first bytes of memory, where the kernel is are identity mapped,
    // Which is true if we're using Limine
    for (uint64_t i = 0; i < HHDM_SIZE; i++) {
        void *virt = (void *) (HHDM_BEGIN + i * 1024ULL * 1024ULL * 1024ULL);
        void *real = (void *) (i * 1024ULL * 1024ULL * 1024ULL);

        uint64_t pml4i = (uint64_t) virt >> 39 & 0x01FF;
        uint64_t pdpei = (uint64_t) virt >> 30 & 0x01FF;

        assert2((uint64_t) pml4 < 0x8000000000000000ULL, "CR3 here must be physical!");
        uint64_t *pml4e = &(pml4[pml4i]);

        if (!(*pml4e & PAGE_PRESENT)) {
            assert2(early_pages_used < EARLY_PAGES_SIZE, "Couldn't get a page for HHDM!");
            uint64_t *newp = early_pages[early_pages_used++];
            for (int i = 0; i < 512; i++)
                newp[i] = PAGE_RW;
            *pml4e = PAGE_RW | PAGE_PRESENT;
            *pml4e |= (uint64_t) KERN_V2P(newp) & (uint64_t) 0x000FFFFFFFFFF000ULL;
        }
        *pml4e |= PAGE_RW | PAGE_PRESENT;
        uint64_t *pdpeb = (uint64_t *) (*pml4e & 0x000FFFFFFFFFF000ULL);
        uint64_t *pdpee = &pdpeb[pdpei];
        assert2((!(*pdpee & PAGE_PRESENT)), "HHDM area is already mapped!");
        *pdpee = PAGE_RW | PAGE_PRESENT | PAGE_PS;
        *pdpee |= (uint64_t) real & (uint64_t) 0x000FFFFFFFFFF000ULL;
    }
    _tlb_flush();
}
