#include <cstddef>
#include <cstdint>
#include <new>

#include "assert.h"
#include "gdt.hpp"
#include "globals.hpp"
#include "idt.hpp"
#include "kmem.hpp"
#include "limine.h"
#include "limine_fb.hpp"
#include "limine_mm.hpp"
#include "limine_modules.hpp"
#include "memman.hpp"
#include "misc.h"
#include "paging.hpp"

AddressSpace *BOOT_AddressSpace;
alignas(AddressSpace) char BOOT_AddressSpace_storage[sizeof(AddressSpace)];

AddressSpace *KERN_AddressSpace;
alignas(AddressSpace) char KERN_AddressSpace_storage[sizeof(AddressSpace)];

extern void kmain();

// Do final preparations in the new address space then call kmain
extern "C" __attribute__((noreturn))
__attribute__((used)) void
real_start() {
    init_kern_heap();
    parse_limine_memmap(limine_mm_entries, limine_mm_count, LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE);
    limine_fb_remap(KERN_AddressSpace);
    kmain();
}

// Set up the address space for the kernel and prepare other structures to work without the bootloader,
// then call real_start with this address space and the new stack.
extern "C" __attribute__((unused)) void _start(void) {
    _sse_setup();
    barrier();
    Arch::GDT::gdt_setup();
    barrier();
    Arch::IDT::idt_init();
    barrier();
    init_serial();
    barrier();
    limine_kern_save_response();
    barrier();
    map_hhdm(get_cr3());
    barrier();
    BOOT_AddressSpace = new (BOOT_AddressSpace_storage) AddressSpace((uint64_t *) HHDM_P2V(get_cr3()));

    limine_fb_save_response(BOOT_AddressSpace);
    limine_mm_save_response();

    parse_limine_memmap(limine_mm_entries, limine_mm_count, LIMINE_MEMMAP_USABLE);

    uint64_t *KERN_AddressSpace_PML4 = static_cast<uint64_t *>(get4k());
    for (int i = 0; i < 512; i++)
        KERN_AddressSpace_PML4[i] = 0x02;
    map_hhdm((uint64_t *) HHDM_V2P(KERN_AddressSpace_PML4));

    KERN_AddressSpace = new (KERN_AddressSpace_storage) AddressSpace(KERN_AddressSpace_PML4);

    // TODO: Accurate kernel length
    for (int i = 0; i < 100000; i++) {
        // FIXME:
        KERN_AddressSpace->map((void *) (kernel_virt_base + i * PAGE_SIZE), (void *) (kernel_phys_base + i * PAGE_SIZE), PAGE_RW);
    }
    limine_modules_remap();

    uint64_t  real_new_cr3  = (uint64_t) HHDM_V2P(KERN_AddressSpace_PML4);
    uint64_t *new_stack_top = &KERN_stack[KERN_STACK_SIZE - 1];                                     // Don't forget in which direction the stack grows...
    new_stack_top           = reinterpret_cast<uint64_t *>(((uint64_t) new_stack_top) & (~0xFULL)); // correct alignment for sse

    barrier();
    __asm__ volatile("movq %[new_stack_top], %%rsp; movq %[real_new_cr3], %%cr3; call real_start"
                     :
                     : [real_new_cr3] "r"(real_new_cr3), [new_stack_top] "r"(new_stack_top));
}