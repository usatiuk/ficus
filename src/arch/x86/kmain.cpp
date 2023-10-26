//
// Created by Stepan Usatiuk on 13.08.2023.
//
#include <cstddef>

#include "LockGuard.hpp"
#include "SkipList.hpp"
#include "String.hpp"
#include "TestTemplates.hpp"
#include "asserts.hpp"
#include "globals.hpp"
#include "kmem.hpp"
#include "limine_fb.hpp"
#include "memman.hpp"
#include "misc.hpp"
#include "mutex.hpp"
#include "rand.h"
#include "syscalls.hpp"
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
    while (1) {
        String buf;
        buf += "=====\n";
        buf += "Free mem: ";
        buf += get_free() * 1024;
        buf += "\n";
        all_tty_putstr(buf.c_str());
        buf = "";

        buf += "Heap allocated: ";
        buf += get_heap_allocated();
        buf += "\n";
        all_tty_putstr(buf.c_str());
        buf = "";

        buf += "Heap used: ";
        buf += get_heap_used();
        buf += "\n";
        buf += "=====\n";
        all_tty_putstr(buf.c_str());
        sleep_self(1000000);
    }
}

void statprinter() {
    SkipList<uint64_t, std::pair<String, uint64_t>> last_times = getTaskTimePerPid();
    std::atomic<uint64_t> last_print_time = micros;
    while (1) {
        sleep_self(1000000);
        uint64_t prev_print_time = last_print_time;
        last_print_time = micros;
        SkipList<uint64_t, std::pair<String, uint64_t>> prev_times = std::move(last_times);
        last_times = getTaskTimePerPid();

        uint64_t slice = last_print_time - prev_print_time;
        if (slice == 0) continue;

        for (const auto &t: prev_times) {
            auto f = last_times.find(t.key);
            if (!f->end && f->key == t.key) {
                assert(f->data.second >= t.data.second);
                String buf;
                buf += "PID: ";
                buf += t.key;
                buf += " ";
                buf += t.data.first;
                buf += " usage: ";
                buf += (((f->data.second - t.data.second) * 100ULL) / slice);
                buf += "%\n";
                all_tty_putstr(buf.c_str());
            } else {
                String buf;
                buf += "PID: ";
                buf += t.key;
                buf += " ";
                buf += t.data.first;
                buf += " dead \n";
                all_tty_putstr(buf.c_str());
            }
        }
    }
}

static Mutex testmutex;

void mtest1() {
    {
        LockGuard l(testmutex);
        all_tty_putstr("Locked1\n");
        sleep_self(100000);
    }
    all_tty_putstr("Unlocked1\n");
    remove_self();
}

void mtest2() {
    {
        LockGuard l(testmutex);
        all_tty_putstr("Locked2\n");
        sleep_self(100000);
    }
    all_tty_putstr("Unlocked2\n");
    remove_self();
}

void mtest3() {
    {
        LockGuard l(testmutex);
        all_tty_putstr("Locked3\n");
        sleep_self(100000);
    }
    all_tty_putstr("Unlocked3\n");
    remove_self();
}

void stress() {
    static std::atomic<int> i = 0;
    int curi = i++;
    if (curi > 1500) remove_self();

    sleep_self(100000 - curi * 10);

    char buf[69];
    itoa(curi, buf, 10);
    //    all_tty_putstr("stress ");
    //    all_tty_putstr(buf);
    //    all_tty_putstr("\n");
    remove_self();
}

void templates_tester() {
    all_tty_putstr("Testing templates\n");
    for (int i = 0; i < 2000; i++)
        test_templates();
    all_tty_putstr("Testing templates OK\n");

    remove_self();
}

void stress_tester() {
    for (int i = 0; i < 2000; i++)
        new_ktask(stress, "stress");

    all_tty_putstr("Finished stress\n");

    remove_self();
}


void user_task() {
    while (true) {
        asm("syscall");
        __builtin_ia32_pause();
    }
}

void ktask_main() {
    struct tty_funcs serial_tty = {.putchar = write_serial};
    add_tty(serial_tty);

    new_ktask(ktask, "one");
    new_ktask(freeprinter, "freeprinter");
    new_ktask(statprinter, "statprinter");
    new_ktask(mtest1, "mtest1");
    new_ktask(mtest2, "mtest2");
    new_ktask(mtest3, "mtest3");
    new_ktask(templates_tester, "templates_tester");
    new_ktask(templates_tester, "templates_tester2");
    new_ktask(stress_tester, "stress_tester");

    new_utask(user_task, "user");

    remove_self();
}

void dummy_task() {
    for (;;) {
        yield_self();
    }
}

extern void (*ctors_begin[])();
extern void (*ctors_end[])();

void kmain() {
    for (void (**ctor)() = ctors_begin; ctor < ctors_end; ctor++)
        (*ctor)();

    init_timer();

    srand(micros);// NOLINT

    new_ktask(ktask_main, "ktask_main");
    new_ktask(dummy_task, "dummy");
    setup_syscalls();
    init_tasks();
    for (;;) {
        __asm__ __volatile__("hlt");
    }
}