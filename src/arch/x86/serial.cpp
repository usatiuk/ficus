//
// Created by Stepan Usatiuk on 12.08.2023.
//

#include "asserts.hpp"

#include <stdint.h>

#include "io.hpp"
#include "task.hpp"

#define PORT 0x3f8        // COM1

int init_serial() {
    outb(PORT + 1, 0x00); // Disable all interrupts
    outb(PORT + 3, 0x80); // Enable DLAB (set baud rate divisor)
    outb(PORT + 0, 0x03); // Set divisor to 3 (lo byte) 38400 baud
    outb(PORT + 1, 0x00); //                  (hi byte)
    outb(PORT + 3, 0x03); // 8 bits, no parity, one stop bit
    outb(PORT + 2, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
    outb(PORT + 4, 0x0B); // IRQs enabled, RTS/DSR set
    outb(PORT + 4, 0x1E); // Set in loopback mode, test the serial chip
    outb(PORT + 0, 0xAE); // Test serial chip (send byte 0xAE and check if serial returns same byte)

    // Check if serial is faulty (i.e: not same byte as sent)
    if (inb(PORT + 0) != 0xAE) {
        return 1;
    }

    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outb(PORT + 4, 0x0F);
    return 0;
}

int serial_received() {
    return inb(PORT + 5) & 1;
}

char read_serial() {
    while (serial_received() == 0) {
        yield_self();
    }

    return inb(PORT);
}

int is_transmit_empty() {
    return inb(PORT + 5) & 0x20;
}

void write_serial(char a) {
    while (is_transmit_empty() == 0) {
        yield_self();
    }

    outb(PORT, a);
}

void write_serial_no_yield(char a) {
    while (is_transmit_empty() == 0) {
        __builtin_ia32_pause();
    }

    outb(PORT, a);
}
void writestr(const char *a) {
    while (*a != '\0') write_serial(*a++);
}

void writestr_no_yield(const char *a) {
    while (*a != '\0') write_serial_no_yield(*a++);
}
