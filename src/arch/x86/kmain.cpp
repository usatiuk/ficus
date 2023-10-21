//
// Created by Stepan Usatiuk on 13.08.2023.
//
#include <cstddef>

#include "globals.hpp"
#include "kmem.hpp"
#include "limine_fb.hpp"
#include "memman.hpp"
#include "misc.hpp"
#include "mutex.hpp"
#include "serial.hpp"
#include "task.hpp"
#include "timer.hpp"
#include "tty.hpp"

void ktask();

void ktask2() {
    // Ensure we got a framebuffer.
    assert2(framebuffer_count >= 1, "No framebuffer!");

    struct limine_framebuffer *framebuffer = &framebuffers[0];

    for (uint32_t c = 0; c < 2; c++) {
        // Note: we assume the framebuffer model is RGB with 32-bit pixels.
        for (size_t i = 0; i < 100; i++) {
            sleep_self(25000);
            uint32_t *fb_ptr = static_cast<uint32_t *>(framebuffer->address);
            fb_ptr[i * (framebuffer->pitch / 4) + i + 100] = c ? 0 : 0xFFFFFF;
        }
    }
    new_ktask(ktask, "one");
    remove_self();
}


void ktask() {
    // Ensure we got a framebuffer.
    assert2(framebuffer_count >= 1, "No framebuffer!");

    struct limine_framebuffer *framebuffer = &framebuffers[0];

    for (uint32_t c = 0; c < 2; c++) {
        // Note: we assume the framebuffer model is RGB with 32-bit pixels.
        for (size_t i = 0; i < 100; i++) {
            sleep_self(25000);
            uint32_t *fb_ptr = static_cast<uint32_t *>(framebuffer->address);
            fb_ptr[i * (framebuffer->pitch / 4) + i] = c ? 0 : 0xFFFFFF;
        }
    }
    new_ktask(ktask2, "two");
    remove_self();
}

void freeprinter() {
    char buf[69];
    while (1) {
        all_tty_putstr("=====\n");
        itoa(get_free(), buf, 10);
        all_tty_putstr("Free mem: ");
        all_tty_putstr(buf);
        write_serial('\n');

        itoa(get_heap_allocated() / 1024, buf, 10);
        all_tty_putstr("Heap allocated: ");
        all_tty_putstr(buf);
        write_serial('\n');

        itoa(get_heap_used() / 1024, buf, 10);
        all_tty_putstr("Heap used: ");
        all_tty_putstr(buf);
        write_serial('\n');
        all_tty_putstr("=====\n");

        sleep_self(1000000);
    }
}

static struct Mutex testmutex;

void mtest1() {
    m_lock(&testmutex);
    all_tty_putstr("Locked1\n");
    sleep_self(100000);
    m_unlock(&testmutex);
    all_tty_putstr("Unlocked1\n");
    remove_self();
}

void mtest2() {
    m_lock(&testmutex);
    all_tty_putstr("Locked2\n");
    sleep_self(100000);
    m_unlock(&testmutex);
    all_tty_putstr("Unlocked2\n");
    remove_self();
}

void mtest3() {
    m_lock(&testmutex);
    all_tty_putstr("Locked3\n");
    sleep_self(100000);
    m_unlock(&testmutex);
    all_tty_putstr("Unlocked3\n");
    remove_self();
}

void stress() {
    static std::atomic<int> i = 0;
    int curi = i++;
    if (curi > 1500) remove_self();

    sleep_self(10000 - curi * 10);

    char buf[69];
    itoa(curi, buf, 10);
    all_tty_putstr("stress ");
    all_tty_putstr(buf);
    all_tty_putstr("\n");
    remove_self();
}

void ktask_main() {

    new_ktask(ktask, "one");
    new_ktask(freeprinter, "freeprinter");
    new_ktask(mtest1, "mtest1");
    new_ktask(mtest2, "mtest2");
    new_ktask(mtest3, "mtest3");

    for (int i = 0; i < 2000; i++)
        new_ktask(stress, "stress");

    all_tty_putstr("Finished stress");

    remove_self();
}

void dummy_task() {
    for (;;) {
        __asm__ __volatile__("hlt");
    }
}

extern void (*ctors_begin[])();
extern void (*ctors_end[])();

void kmain() {
    struct tty_funcs serial_tty = {.putchar = write_serial};
    add_tty(serial_tty);

    for (void (**ctor)() = ctors_begin; ctor < ctors_end; ctor++)
        (*ctor)();

    init_timer();
    new_ktask(ktask_main, "ktask_main");
    new_ktask(dummy_task, "dummy");
    init_tasks();
    for (;;) {
        __asm__ __volatile__("hlt");
    }
}