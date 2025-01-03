//
// Created by Stepan Usatiuk on 14.08.2023.
//

#include "timer.hpp"

#include "idt.hpp"
#include "io.hpp"

volatile std::atomic<uint64_t> ticks;
volatile std::atomic<uint64_t> micros;

unsigned read_pit_count(void) {
    unsigned count = 0;

    // Disable interrupts
    //    cli();

    // al = channel in bits 6 and 7, remaining bits clear
    outb(0x43, 0b0000000);

    count = inb(0x40);       // Low byte
    count |= inb(0x40) << 8; // High byte

    return count;
}

void set_pit_count(unsigned count) {
    // Disable interrupts
    //    cli();

    // Set low byte
    outb(0x40, count & 0xFF);          // Low byte
    outb(0x40, (count & 0xFF00) >> 8); // High byte
    return;
}

// Very rough but I don't care right now
// About 1000 HZ freq
#define RELOAD_VAL      1193
#define FREQ            (1193182 / (RELOAD_VAL))
#define MICROS_PER_TICK (1000000 / (FREQ))

static_assert(MICROS_PER_TICK >= 1);

void init_timer() {
    outb(0x43, 0b00110100);
    set_pit_count(RELOAD_VAL);
    Arch::IDT::IRQ_clear_mask(0);
}

void timer_tick() {
    ticks.fetch_add(1);
    micros.fetch_add(MICROS_PER_TICK);
}