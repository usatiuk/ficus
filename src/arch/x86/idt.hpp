#ifndef FICUS_IDT_H
#define FICUS_IDT_H

#include <cstddef>
#include <cstdint>

namespace Arch::IDT {
    static constexpr int kPIC1         = 0x20; /* IO base address for master PIC */
    static constexpr int kPIC2         = 0xA0; /* IO base address for slave PIC */
    static constexpr int kPIC1_COMMAND = kPIC1;
    static constexpr int kPIC1_DATA    = (kPIC1 + 1);
    static constexpr int kPIC2_COMMAND = kPIC2;
    static constexpr int kPIC2_DATA    = (kPIC2 + 1);
    static constexpr int kPIC_EOI      = 0x20;    /* End-of-interrupt command code */

    static constexpr int kICW1_ICW4      = 0x01;  /* Indicates that ICW4 will be present */
    static constexpr int kICW1_SINGLE    = 0x02;  /* Single (cascade) mode */
    static constexpr int kICW1_INTERVAL4 = 0x04;  /* Call address interval 4 (8) */
    static constexpr int kICW1_LEVEL     = 0x08;  /* Level triggered (edge) mode */
    static constexpr int kICW1_INIT      = 0x10;  /* Initialization - required! */

    static constexpr int kICW4_8086       = 0x01; /* 8086/88 (MCS-80/85) mode */
    static constexpr int kICW4_AUTO       = 0x02; /* Auto (normal) EOI */
    static constexpr int kICW4_BUF_SLAVE  = 0x08; /* Buffered mode/slave */
    static constexpr int kICW4_BUF_MASTER = 0x0C; /* Buffered mode/master */
    static constexpr int kICW4_SFNM       = 0x10; /* Special fully nested (not) */

    static constexpr int kPIC_READ_IRR = 0x0a;    /* OCW3 irq ready next CMD read */
    static constexpr int kPIC_READ_ISR = 0x0b;    /* OCW3 irq service next CMD read */

    static constexpr int kPIC1_OFFSET = 0x20;
    static constexpr int kPIC2_OFFSET = 0x28;

    void     PIC_sendEOI(unsigned char irq);
    void     PIC_init();
    void     IRQ_set_mask(unsigned char IRQline);
    void     IRQ_clear_mask(unsigned char IRQline);
    uint16_t pic_get_irr(void);
    uint16_t pic_get_isr(void);

    struct IdtEntry {
        uint16_t isr_low;    // The lower 16 bits of the ISR's address
        uint16_t kernel_cs;  // The GDT segment selector that the CPU will load into CS before calling the ISR
        uint8_t  ist;        // The IST in the TSS that the CPU will load into RSP; set to zero for now
        uint8_t  attributes; // Type and attributes; see the IDT page
        uint16_t isr_mid;    // The higher 16 bits of the lower 32 bits of the ISR's address
        uint32_t isr_high;   // The higher 32 bits of the ISR's address
        uint32_t reserved;   // Set to zero
    } __attribute__((packed));

    struct Idtr {
        uint16_t limit;
        uint64_t base;
    } __attribute__((packed));


    extern "C" void exception_handler_err(uint64_t code);
    extern "C" void exception_handler_no_err(void);

    void idt_set_descriptor(uint8_t vector, void (*isr)(), uint8_t flags);

    void idt_init();

    extern "C" void (*isr_stub_table[])();
} // namespace Arch::IDT

using int_handler_t = void (*)(void *);

void attach_interrupt(unsigned num, int_handler_t handler, void *arg);

#endif