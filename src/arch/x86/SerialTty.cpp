//
// Created by Stepan Usatiuk on 26.11.2023.
//

#include "SerialTty.hpp"
#include "LockGuard.hpp"
#include "idt.hpp"
#include "io.hpp"

#define PORT 0x3f8 // COM1

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
    mutex.lock();
    while (true) {
        bool read_something = false;
        int  r              = read();
        while (r != -1) {
            read_something = true;
            if (!buf.full())
                buf.push_back((char) r);
            r = read();
        }
        if (read_something)
            readercv.notify_all();
        isrcv.wait(mutex);
    }
    mutex.unlock();
}

SerialTty::SerialTty() : Tty() {
    outb(PORT + 3, 0x00); // Disable DLAB
    outb(PORT + 1, 0x01); // Enable data available interrupt

    Task *task       = new Task(Task::TaskMode::TASKMODE_KERN, (void (*)(void))(&SerialTty::this_pooler), "serialpooler");
    task->_frame.rdi = reinterpret_cast<uint64_t>(this);
    task->start();

    attach_interrupt(4, &SerialTty::isr, this);
    IRQ_clear_mask(4);
}
void SerialTty::isr(void *tty) {
    ((SerialTty *) tty)->this_isr();
}


void SerialTty::this_isr() {
    isrcv.notify_one();
}

char SerialTty::readchar() {
    mutex.lock();
    if (buf.empty()) {
        readercv.wait(mutex);
    }
    assert(!buf.empty());
    char ret = buf.pop_back();
    mutex.unlock();
    return ret;
}
