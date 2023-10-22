#ifndef OS1_IDT_H
#define OS1_IDT_H

#include <stddef.h>
#include <stdint.h>

#define PIC1 0x20 /* IO base address for master PIC */
#define PIC2 0xA0 /* IO base address for slave PIC */
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)
#define PIC_EOI 0x20 /* End-of-interrupt command code */

#define ICW1_ICW4 0x01      /* Indicates that ICW4 will be present */
#define ICW1_SINGLE 0x02    /* Single (cascade) mode */
#define ICW1_INTERVAL4 0x04 /* Call address interval 4 (8) */
#define ICW1_LEVEL 0x08     /* Level triggered (edge) mode */
#define ICW1_INIT 0x10      /* Initialization - required! */

#define ICW4_8086 0x01       /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO 0x02       /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE 0x08  /* Buffered mode/slave */
#define ICW4_BUF_MASTER 0x0C /* Buffered mode/master */
#define ICW4_SFNM 0x10       /* Special fully nested (not) */

#define PIC_READ_IRR 0x0a /* OCW3 irq ready next CMD read */
#define PIC_READ_ISR 0x0b /* OCW3 irq service next CMD read */

#define PIC1_OFFSET 0x20
#define PIC2_OFFSET 0x28

void PIC_sendEOI(unsigned char irq);
void PIC_init();
void IRQ_set_mask(unsigned char IRQline);
void IRQ_clear_mask(unsigned char IRQline);
uint16_t pic_get_irr(void);
uint16_t pic_get_isr(void);

typedef struct {
    uint16_t isr_low;  // The lower 16 bits of the ISR's address
    uint16_t kernel_cs;// The GDT segment selector that the CPU will load into CS before calling the ISR
    uint8_t ist;       // The IST in the TSS that the CPU will load into RSP; set to zero for now
    uint8_t attributes;// Type and attributes; see the IDT page
    uint16_t isr_mid;  // The higher 16 bits of the lower 32 bits of the ISR's address
    uint32_t isr_high; // The higher 32 bits of the ISR's address
    uint32_t reserved; // Set to zero
} __attribute__((packed)) idt_entry_t;

typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) idtr_t;

#define IDT_GUARD 0xdeadbe3fdeadb3efULL

// Assuming the compiler understands that this is pushed on the stack in the correct order
struct task_frame {
    uint64_t guard;

    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;

    uint64_t rdi;
    uint64_t rsi;
    uint64_t rbp;
    uint64_t rbx;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rax;

    uint64_t ip;
    uint64_t cs;
    uint64_t flags;
    uint64_t sp;
    uint64_t ss;

} __attribute__((packed));

extern "C" void exception_handler(void);

void idt_set_descriptor(uint8_t vector, void (*isr)(), uint8_t flags);

void idt_init(void);

extern void (*isr_stub_table[])();

#endif