//
// Created by Stepan Usatiuk on 28.04.2024.
//

#include "PS2Keyboard.hpp"

#include <io.hpp>

#define PORTD 0x60 // COM1
#define PORTS 0x64 // COM1
#define PORTC 0x64 // COM1

static void wait_in() {
    while ((inb(PORTS) & 1) == 0) {
        __builtin_ia32_pause();
    }
}

static void wait_out() {
    while (inb(PORTS) & 2) {
        __builtin_ia32_pause();
    }
}

static void outb_wait(uint16_t port, uint8_t val) {
    wait_out();
    outb(port, val);
}

static uint8_t inb_wait(uint16_t port) {
    wait_in();
    return inb(port);
}

void PS2Keyboard::this_pooler() {
    mutex.lock();
    while (true) {
        isrcv.wait(mutex);
        wait_in();

        bool read_something = false;

        while ((inb(PORTS) & 1) != 0) {
            int  read = inb_wait(PORTD);
            char r    = -1;
            if (read == 0xF0) {
                inb_wait(PORTD);
                continue;
            }
            switch (read) {
                case 0x1C:
                    r = 'a';
                    break;
                case 0x24:
                    r = 'e';
                    break;
                case 0x2C:
                    r = 't';
                    break;
                case 0x34:
                    r = 'g';
                    break;
                case 0x3C:
                    r = 'u';
                    break;
                case 0x44:
                    r = 'o';
                    break;
                case 0x15:
                    r = 'q';
                    break;
                case 0x1D:
                    r = 'w';
                    break;
                case 0x21:
                    r = 'c';
                    break;
                case 0x29:
                    r = ' ';
                    break;
                case 0x2D:
                    r = 'r';
                    break;
                case 0x31:
                    r = 'n';
                    break;
                case 0x35:
                    r = 'y';
                    break;
                case 0x4D:
                    r = 'p';
                    break;
                case 0x1A:
                    r = 'z';
                    break;
                case 0x22:
                    r = 'x';
                    break;
                case 0x2A:
                    r = 'v';
                    break;
                case 0x32:
                    r = 'b';
                    break;
                case 0x3A:
                    r = 'm';
                    break;
                case 0x42:
                    r = 'k';
                    break;
                case 0x1B:
                    r = 's';
                    break;
                case 0x23:
                    r = 'd';
                    break;
                case 0x2B:
                    r = 'f';
                    break;
                case 0x33:
                    r = 'h';
                    break;
                case 0x3B:
                    r = 'j';
                    break;
                case 0x43:
                    r = 'i';
                    break;
                case 0x4B:
                    r = 'l';
                    break;

                case 0x54:
                    r = '(';
                    break;
                case 0x5B:
                    r = ')';
                    break;
                case 0x5A: {
                    read_something = true;
                    if (!buf.full())
                        buf.push_back((char) '\n');
                    if (!buf.full())
                        buf.push_back((char) '\r');
                    break;
                }
                case 0x16:
                    r = '1';
                    break;
                case 0x1E:
                    r = '2';
                    break;
                case 0x26:
                    r = '3';
                    break;
                case 0x25:
                    r = '4';
                    break;
                case 0x2E:
                    r = '5';
                    break;
                case 0x36:
                    r = '6';
                    break;
                case 0x3D:
                    r = '7';
                    break;
                case 0x3E:
                    r = '8';
                    break;
                case 0x46:
                    r = '9';
                    break;
            }

            if (r != -1) {
                read_something = true;
                if (!buf.full())
                    buf.push_back((char) r);
            }
        }

        if (read_something)
            readercv.notify_all();
    }
    mutex.unlock();
}

PS2Keyboard::PS2Keyboard() {
    outb_wait(PORTC, 0xAD); // Disable 1
    outb_wait(PORTC, 0xA7); // Disable 2
    inb(PORTD);
    outb_wait(PORTC, 0x20); // Read conf
    uint8_t old_conf = inb_wait(PORTD);
    old_conf &= ~(1 << 0 | 1 << 1 | 1 << 6);
    outb_wait(PORTC, 0x60); // Write conf
    outb_wait(PORTD, old_conf);

    outb_wait(PORTC, 0xAA);
    assert(inb_wait(PORTD) == 0x55);
    outb_wait(PORTC, 0xAB);
    assert(inb_wait(PORTD) == 0x00);
    outb_wait(PORTC, 0xAE);

    outb_wait(PORTC, 0x20); // Read conf
    old_conf = inb_wait(PORTD);
    old_conf |= 1;          // Enable IRQ1
    outb_wait(PORTC, 0x60); // Write conf
    outb_wait(PORTD, old_conf);

    outb_wait(PORTD, 0xFF); // Reset
    assert(inb_wait(PORTD) == 0xFA);
    outb_wait(PORTD, 0xF6); // Set default
    assert(inb_wait(PORTD) == 0xFA);

    outb_wait(PORTD, 0xF0); // Scancode
    assert(inb_wait(PORTD) == 0xFA);

    outb_wait(PORTD, 2);    // Scancode 2
    assert(inb_wait(PORTD) == 0xFA);


    outb_wait(PORTD, 0xF4); // Scan
    assert(inb_wait(PORTD) == 0xFA);

    for (int i = 0; i < 256; i++) states[i] = false;

    Task *task       = new Task(Task::TaskMode::TASKMODE_KERN, (void (*)(void))(&PS2Keyboard::this_pooler), "ps2kbd");
    task->_frame.rdi = reinterpret_cast<uint64_t>(this);
    task->start();

    attach_interrupt(1, &PS2Keyboard::isr, this);
    Arch::IDT::IRQ_clear_mask(1);
}
void PS2Keyboard::isr(void *tty) {
    ((PS2Keyboard *) tty)->this_isr();
}

void PS2Keyboard::this_isr() {
    isrcv.notify_one();
}

char PS2Keyboard::readchar() {
    mutex.lock();
    while (buf.empty()) {
        readercv.wait(mutex);
    }
    char ret = buf.pop_back();
    mutex.unlock();
    return ret;
}
