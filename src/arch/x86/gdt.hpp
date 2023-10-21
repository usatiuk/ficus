#ifndef OS1_GDT_H
#define OS1_GDT_H

#include <cstdint>

struct gdt_entry_bits {
    unsigned int limit_low : 16;
    unsigned int base_low : 24;
    unsigned int accessed : 1;
    unsigned int read_write : 1;            // readable for code, writable for data
    unsigned int conforming_expand_down : 1;// conforming for code, expand down for data
    unsigned int code : 1;                  // 1 for code, 0 for data
    unsigned int code_data_segment : 1;     // should be 1 for everything but TSS and LDT
    unsigned int DPL : 2;                   // privilege level
    unsigned int present : 1;
    unsigned int limit_high : 4;
    unsigned int available : 1;// only used in software; has no effect on hardware
    unsigned int long_mode : 1;
    unsigned int big : 1; // 32-bit opcodes for code, uint32_t stack for data
    unsigned int gran : 1;// 1 to use 4k page addressing, 0 for byte addressing
    unsigned int base_high : 8;
} __attribute__((packed));

struct gdt_tss_entry_bits {
    unsigned int limit_low : 16;
    unsigned int base_low : 24;
    unsigned int type : 4;
    unsigned int zero : 1;
    unsigned int DPL : 2;
    unsigned int present : 1;
    unsigned int limit_high : 4;
    unsigned int available : 1;
    unsigned int unused : 2;
    unsigned int gran : 1;
    uint64_t base_high : 40;
    unsigned int zeros : 32;
} __attribute__((packed));

struct tss_entry_struct {
    uint32_t reserved;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved2;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved3;
    uint32_t reserved4;
} __attribute__((packed));

extern "C" void _gdt_setup();
void gdt_setup();

extern volatile struct gdt_entry_bits gdt_null;
extern volatile struct gdt_entry_bits gdt_code_16;
extern volatile struct gdt_entry_bits gdt_data_16;
extern volatile struct gdt_entry_bits gdt_code_32;
extern volatile struct gdt_entry_bits gdt_data_32;
extern volatile struct gdt_entry_bits gdt_code;
extern volatile struct gdt_entry_bits gdt_data;
extern volatile struct gdt_entry_bits gdt_code_user;
extern volatile struct gdt_entry_bits gdt_data_user;
extern volatile struct gdt_tss_entry_bits gdt_tss;
extern volatile struct gdt_tss_entry_bits gdt_tss_user;

extern volatile struct gdt_entry_bits gdt_end;/// It is not a pointer!

extern struct {
    uint16_t limit;
    uint64_t base;
} gdtr;

#define GDTSEL(x) (((uint64_t) &x) - ((uint64_t) &gdt_null))

#endif