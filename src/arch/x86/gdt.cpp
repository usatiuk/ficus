//
// Created by Stepan Usatiuk on 13.08.2023.
//

#include "gdt.hpp"
#include "misc.hpp"

static struct tss_entry_struct tss_entry;
static struct tss_entry_struct tss_entry_user;

#define INT_STACK_SIZE 16384
#define RSP_STACK_SIZE 16384
static uint64_t int_stack[INT_STACK_SIZE];
static uint64_t rsp_stack[RSP_STACK_SIZE];

void gdt_setup() {
    uint32_t tss_limit = sizeof(tss_entry);
    uint64_t tss_base = (uint64_t) &tss_entry;

    gdt_tss.limit_low = tss_limit & 0xFFFF;
    gdt_tss.base_low = tss_base & 0xFFFFFF;
    gdt_tss.type = 0b1001;// Available 64 bit TSS
    gdt_tss.zero = 0;
    gdt_tss.DPL = 0;
    gdt_tss.present = 1;
    gdt_tss.limit_high = (tss_limit >> 16) & 0xF;
    gdt_tss.available = 0;
    gdt_tss.unused = 0;
    gdt_tss.gran = 0;
    gdt_tss.base_high = (tss_base >> 24) & 0xFFFFFFFFFF;

    tss_entry.ist1 = (((uintptr_t) int_stack + (INT_STACK_SIZE - 9) - 1) & (~0xFULL)) + 8;
    if ((tss_entry.ist1 & 0xFULL) != 8) _hcf();
    tss_entry.rsp0 = (((uintptr_t) rsp_stack + (RSP_STACK_SIZE - 9) - 1) & (~0xFULL)) + 8;
    if ((tss_entry.rsp0 & 0xFULL) != 8) _hcf();

    barrier();// The asm function might clobber registers
    _gdt_setup();
}