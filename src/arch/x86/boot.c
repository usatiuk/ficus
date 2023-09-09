#include <stddef.h>
#include <stdint.h>

#include "gdt.h"
#include "globals.h"
#include "idt.h"
#include "kmem.h"
#include "limine.h"
#include "limine_fb.h"
#include "limine_mm.h"
#include "memman.h"
#include "misc.h"
#include "paging.h"
#include "serial.h"

struct AddressSpace BOOT_AddressSpace;

extern void kmain();

// Do final preparations in the new address space then call kmain
__attribute__((noreturn))
__attribute__((used))
void real_start() {
    parse_limine_memmap(limine_mm_entries, limine_mm_count, LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE);
    limine_fb_remap(KERN_AddressSpace);
    init_kern_heap();
    kmain();
}

// Set up the address space for the kernel and prepare other structures to work without the bootloader,
// then call real_start with this address space and the new stack.
void _start(void) {
    _sse_setup();
    barrier();
    gdt_setup();
    barrier();
    idt_init();
    barrier();
    init_serial();
    barrier();
    limine_kern_save_response();
    barrier();
    map_hddm(get_cr3());
    barrier();
    BOOT_AddressSpace.PML4 = (uint64_t *) HHDM_P2V(get_cr3());

    limine_fb_save_response(&BOOT_AddressSpace);
    limine_mm_save_response();

    parse_limine_memmap(limine_mm_entries, limine_mm_count, LIMINE_MEMMAP_USABLE);

    KERN_AddressSpace = get4k();
    assert2(!init_addr_space(KERN_AddressSpace), "Couldn't init kernel address space!");

    for (int i = 0; i < 512; i++)
        ((struct AddressSpace *) (KERN_AddressSpace))->PML4[i] = 0x02;

    map_hddm((uint64_t *) HHDM_V2P(((struct AddressSpace *) (KERN_AddressSpace))->PML4));

    // TODO: Accurate kernel length
    for (int i = 0; i < 100000; i++) {
        map((void *) (kernel_virt_base + i * 4096), (void *) (kernel_phys_base + i * 4096), PAGE_RW, KERN_AddressSpace);
    }

    uint64_t real_new_cr3 = (uint64_t) HHDM_V2P(((struct AddressSpace *) (KERN_AddressSpace))->PML4);
    uint64_t *new_stack_top = &KERN_stack[KERN_STACK_SIZE - 1];// Don't forget in which direction the stack grows...

    barrier();
    __asm__ volatile("movq %[new_stack_top], %%rsp; movq %[real_new_cr3], %%cr3; call real_start"
                     :
                     : [real_new_cr3] "r"(real_new_cr3), [new_stack_top] "r"(new_stack_top));
}