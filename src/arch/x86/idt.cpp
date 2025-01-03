#include "idt.hpp"

#include "assert.h"
#include "gdt.hpp"
#include "io.hpp"
#include "misc.h"
#include "task.hpp"
#include "task_arch.hpp"
#include "timer.hpp"

namespace Arch::IDT {

    __attribute__((aligned(0x10))) static IdtEntry idt[256]; // Create an array of IDT entries; aligned for performance

    //
    static Idtr idtr;

    extern "C" void pic1_irq_0();
    extern "C" void pic1_irq_1();
    extern "C" void pic1_irq_2();
    extern "C" void pic1_irq_3();
    extern "C" void pic1_irq_4();
    extern "C" void pic1_irq_5();
    extern "C" void pic1_irq_6();
    extern "C" void pic1_irq_7();

    extern "C" void pic2_irq_0();
    extern "C" void pic2_irq_1();
    extern "C" void pic2_irq_2();
    extern "C" void pic2_irq_3();
    extern "C" void pic2_irq_4();
    extern "C" void pic2_irq_5();
    extern "C" void pic2_irq_6();
    extern "C" void pic2_irq_7();


    void idt_set_descriptor(uint8_t vector, void (*isr)(), uint8_t flags) {
        IdtEntry *descriptor = &idt[vector];

        descriptor->isr_low    = (uint64_t) isr & 0xFFFF;
        descriptor->kernel_cs  = Arch::GDT::gdt_code.selector();
        descriptor->ist        = 0;
        descriptor->attributes = flags;
        descriptor->isr_mid    = ((uint64_t) isr >> 16) & 0xFFFF;
        descriptor->isr_high   = ((uint64_t) isr >> 32) & 0xFFFFFFFF;
        descriptor->reserved   = 0;
    }

    void idt_init() {
        idtr.base  = (uintptr_t) &idt[0];
        idtr.limit = (uint16_t) ((uint64_t) &idt[255] - (uint64_t) &idt[0]);

        for (uint8_t vector = 0; vector < 32; vector++) {
            idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
        }

        idt_set_descriptor(kPIC1_OFFSET + 0, pic1_irq_0, 0x8e);
        idt_set_descriptor(kPIC1_OFFSET + 1, pic1_irq_1, 0x8e);
        idt_set_descriptor(kPIC1_OFFSET + 2, pic1_irq_2, 0x8e);
        idt_set_descriptor(kPIC1_OFFSET + 3, pic1_irq_3, 0x8e);
        idt_set_descriptor(kPIC1_OFFSET + 4, pic1_irq_4, 0x8e);
        idt_set_descriptor(kPIC1_OFFSET + 5, pic1_irq_5, 0x8e);
        idt_set_descriptor(kPIC1_OFFSET + 6, pic1_irq_6, 0x8e);
        idt_set_descriptor(kPIC1_OFFSET + 7, pic1_irq_7, 0x8e);

        idt_set_descriptor(kPIC2_OFFSET + 0, pic2_irq_0, 0x8e);
        idt_set_descriptor(kPIC2_OFFSET + 1, pic2_irq_1, 0x8e);
        idt_set_descriptor(kPIC2_OFFSET + 2, pic2_irq_2, 0x8e);
        idt_set_descriptor(kPIC2_OFFSET + 3, pic2_irq_3, 0x8e);
        idt_set_descriptor(kPIC2_OFFSET + 4, pic2_irq_4, 0x8e);
        idt_set_descriptor(kPIC2_OFFSET + 5, pic2_irq_5, 0x8e);
        idt_set_descriptor(kPIC2_OFFSET + 6, pic2_irq_6, 0x8e);
        idt_set_descriptor(kPIC2_OFFSET + 7, pic2_irq_7, 0x8e);

        barrier();
        __asm__ volatile("lidt %0"
                         :
                         : "m"(idtr)); // load the new IDT
        __asm__ volatile("sti");       // set the interrupt flag
        barrier();

        PIC_init();
    }

    void PIC_sendEOI(unsigned char irq) {
        if (irq >= 8)
            outb(kPIC2_COMMAND, kPIC_EOI);

        outb(kPIC1_COMMAND, kPIC_EOI);
    }

    void PIC_init() {
        unsigned char a1, a2;

        a1 = inb(kPIC1_DATA);                         // save masks
        a2 = inb(kPIC2_DATA);

        outb(kPIC1_COMMAND, kICW1_INIT | kICW1_ICW4); // starts the initialization sequence (in cascade mode)
        io_wait();
        outb(kPIC2_COMMAND, kICW1_INIT | kICW1_ICW4);
        io_wait();
        outb(kPIC1_DATA, kPIC1_OFFSET); // ICW2: Master PIC vector offset
        io_wait();
        outb(kPIC2_DATA, kPIC2_OFFSET); // ICW2: Slave PIC vector offset
        io_wait();
        outb(kPIC1_DATA, 4);            // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
        io_wait();
        outb(kPIC2_DATA, 2);            // ICW3: tell Slave PIC its cascade identity (0000 0010)
        io_wait();

        outb(kPIC1_DATA, kICW4_8086);   // ICW4: have the PICs use 8086 mode (and not 8080 mode)
        io_wait();
        outb(kPIC2_DATA, kICW4_8086);
        io_wait();

        outb(kPIC1_DATA, a1); // restore saved masks.
        outb(kPIC2_DATA, a2);
    }
    void IRQ_set_mask(unsigned char IRQline) {
        uint16_t port;
        uint8_t  value;

        if (IRQline < 8) {
            port = kPIC1_DATA;
        } else {
            port = kPIC2_DATA;
            IRQline -= 8;
        }
        value = inb(port) | (1 << IRQline);
        outb(port, value);
    }

    void IRQ_clear_mask(unsigned char IRQline) {
        uint16_t port;
        uint8_t  value;

        if (IRQline < 8) {
            port = kPIC1_DATA;
        } else {
            port = kPIC2_DATA;
            IRQline -= 8;
        }
        value = inb(port) & ~(1 << IRQline);
        outb(port, value);
    }


    /* Helper func */
    static uint16_t __pic_get_irq_reg(int ocw3) {
        /* OCW3 to PIC CMD to get the register values.  PIC2 is chained, and
    * represents IRQs 8-15.  PIC1 is IRQs 0-7, with 2 being the chain */
        outb(kPIC1_COMMAND, ocw3);
        outb(kPIC2_COMMAND, ocw3);
        return (inb(kPIC2_COMMAND) << 8) | inb(kPIC1_COMMAND);
    }

    /* Returns the combined value of the cascaded PICs irq request register */
    uint16_t pic_get_irr(void) {
        return __pic_get_irq_reg(kPIC_READ_IRR);
    }

    /* Returns the combined value of the cascaded PICs in-service register */
    uint16_t pic_get_isr(void) {
        return __pic_get_irq_reg(kPIC_READ_ISR);
    }

    static int_handler_t handlers[256];
    static void         *handlers_args[256];

    // TODO: guarantee alignment in the asm part
    extern "C" __attribute__((force_align_arg_pointer)) void pic1_irq_real_0(TaskFrame *frame) {
        timer_tick();
        Scheduler::switch_task(frame);
        PIC_sendEOI(0);
    }
    extern "C" __attribute__((force_align_arg_pointer)) void pic1_irq_real_1() {
        if (handlers[1] != nullptr) {
            handlers[1](handlers_args[1]);
        }
        PIC_sendEOI(1);
    }
    extern "C" void pic1_irq_real_2() {
        _hcf();
        PIC_sendEOI(2);
    }
    extern "C" void pic1_irq_real_3() {
        PIC_sendEOI(3);
    }
    extern "C" __attribute__((force_align_arg_pointer)) void pic1_irq_real_4() {
        if (handlers[4] != nullptr) {
            handlers[4](handlers_args[4]);
        }
        PIC_sendEOI(4);
    }
    extern "C" void pic1_irq_real_5() {
        PIC_sendEOI(5);
    }
    extern "C" void pic1_irq_real_6() {
        PIC_sendEOI(6);
    }
    extern "C" void pic1_irq_real_7() {
        int irr = pic_get_irr();
        if (!(irr & 0x80)) return;
        PIC_sendEOI(7);
    }

    extern "C" void pic2_irq_real_0() {
        PIC_sendEOI(8);
    }
    extern "C" void pic2_irq_real_1() {
        PIC_sendEOI(9);
    }
    extern "C" void pic2_irq_real_2() {
        PIC_sendEOI(10);
    }
    extern "C" void pic2_irq_real_3() {
        PIC_sendEOI(11);
    }
    extern "C" void pic2_irq_real_4() {
        PIC_sendEOI(12);
    }
    extern "C" void pic2_irq_real_5() {
        PIC_sendEOI(13);
    }
    extern "C" void pic2_irq_real_6() {
        PIC_sendEOI(14);
    }
    extern "C" void pic2_irq_real_7() {
        // Probaby wrong
        int irr = pic_get_irr();
        if (!(irr & (0x80 << 8))) {
            outb(kPIC1_COMMAND, kPIC_EOI);
            return;
        }

        PIC_sendEOI(15);
    }

} // namespace Arch::IDT

void attach_interrupt(unsigned num, int_handler_t handler, void *firstarg) {
    Arch::IDT::handlers[num]      = handler;
    Arch::IDT::handlers_args[num] = firstarg;
}
