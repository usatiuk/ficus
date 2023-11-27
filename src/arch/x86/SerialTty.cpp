//
// Created by Stepan Usatiuk on 26.11.2023.
//

#include "SerialTty.hpp"
#include "LockGuard.hpp"
#include "idt.hpp"
#include "io.hpp"

#define PORT 0x3f8// COM1

void SerialTty::putchar(char c) {
    LockGuard guard(mutex);
    write_serial(c);
}

void SerialTty::putstr(const char *str) {
    LockGuard guard(mutex);
    writestr(str);
}
static int read() {
    if (!(inb(PORT + 5) & 1)) return -1;
    return inb(PORT);
}

void SerialTty::this_pooler() {
    while (true) {
        sleep_self(10000);
        if (intflag != 0) {
            if (mutex.try_lock()) {
                intflag = 0;
                int r = read();
                while (r != -1) {
                    buf.push_back((char) r);
                    r = read();
                }
                cv.notify_one();
                mutex.unlock();
            }
        }
    }
}

SerialTty::SerialTty() : Tty() {
    outb(PORT + 3, 0x00);// Disable DLAB
    outb(PORT + 1, 0x01);// Enable data available interrupt

    Task *task = new_ktask((void (*)(void))(&SerialTty::this_pooler), "serialpooler", false);
    task->frame.rdi = reinterpret_cast<uint64_t>(this);
    start_task(task);

    attach_interrupt(4, &SerialTty::isr, this);
    IRQ_clear_mask(4);
}
void SerialTty::isr(void *tty) {
    ((SerialTty *) tty)->this_isr();
}


void SerialTty::this_isr() {
    intflag.fetch_add(1);
}


char SerialTty::readchar() {
    mutex.lock();
    if (buf.empty()) {
        cv.wait(mutex);
    }
    assert(!buf.empty());
    char ret = buf.pop_back();
    mutex.unlock();
    return ret;
}
