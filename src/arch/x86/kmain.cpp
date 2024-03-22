//
// Created by Stepan Usatiuk on 13.08.2023.
//
#include <cstddef>

#include "BytesFormatter.hpp"
#include "LockGuard.hpp"
#include "MemFs.hpp"
#include "MountTable.hpp"
#include "SerialTty.hpp"
#include "SkipList.hpp"
#include "String.hpp"
#include "TestTemplates.hpp"
#include "TtyManager.hpp"
#include "VFSGlobals.hpp"
#include "VFSTester.hpp"
#include "VMA.hpp"
#include "asserts.hpp"
#include "globals.hpp"
#include "kmem.hpp"
#include "limine_fb.hpp"
#include "limine_modules.hpp"
#include "memman.hpp"
#include "misc.hpp"
#include "mutex.hpp"
#include "rand.h"
#include "syscalls.hpp"
#include "syscalls_interface.h"
#include "task.hpp"
#include "timer.hpp"

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
        buf += BytesFormatter::formatStr(get_free() * 1024);
        buf += "\n";
        GlobalTtyManager.all_tty_putstr(buf.c_str());
        buf = "";

        buf += "Heap allocated: ";
        buf += BytesFormatter::formatStr(get_heap_allocated());
        buf += "\n";
        GlobalTtyManager.all_tty_putstr(buf.c_str());
        buf = "";

        buf += "Heap used: ";
        buf += BytesFormatter::formatStr(get_heap_used());
        buf += "\n";
        buf += "=====\n";
        GlobalTtyManager.all_tty_putstr(buf.c_str());
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
                GlobalTtyManager.all_tty_putstr(buf.c_str());
            } else {
                String buf;
                buf += "PID: ";
                buf += t.key;
                buf += " ";
                buf += t.data.first;
                buf += " dead \n";
                GlobalTtyManager.all_tty_putstr(buf.c_str());
            }
        }
    }
}

static Mutex testmutex;

void mtest1() {
    {
        LockGuard l(testmutex);
        GlobalTtyManager.all_tty_putstr("Locked1\n");
        sleep_self(100000);
    }
    GlobalTtyManager.all_tty_putstr("Unlocked1\n");
    remove_self();
}

void mtest2() {
    {
        LockGuard l(testmutex);
        GlobalTtyManager.all_tty_putstr("Locked2\n");
        sleep_self(100000);
    }
    GlobalTtyManager.all_tty_putstr("Unlocked2\n");
    remove_self();
}

void mtest3() {
    {
        LockGuard l(testmutex);
        GlobalTtyManager.all_tty_putstr("Locked3\n");
        sleep_self(100000);
    }
    GlobalTtyManager.all_tty_putstr("Unlocked3\n");
    remove_self();
}

void stress() {
    static std::atomic<int> i = 0;
    int curi = i++;
    if (curi > 1500) remove_self();

    sleep_self(100000 - curi * 10);

    char buf[69];
    itoa(curi, buf, 10);
    //    GlobalTtyManager.all_tty_putstr("stress ");
    //    GlobalTtyManager.all_tty_putstr(buf);
    //    GlobalTtyManager.all_tty_putstr("\n");
    remove_self();
}

void templates_tester() {
    GlobalTtyManager.all_tty_putstr("Testing templates\n");
    for (int i = 0; i < 2000; i++)
        test_templates();
    GlobalTtyManager.all_tty_putstr("Testing templates OK\n");

    remove_self();
}

void stress_tester() {
    for (int i = 0; i < 2000; i++)
        new_ktask(stress, "stress");

    GlobalTtyManager.all_tty_putstr("Finished stress\n");

    remove_self();
}


void user_task() {
    while (true) {
        putchar('h');
        putchar('i');
        putchar('\n');
        sleep(100000);
    }
}

void vfs_tester() {
    VFSTester vfsTester;
    vfsTester.test();
    remove_self();
}

void ktask_main() {
    GlobalTtyManager.add_tty(new SerialTty());

    new_ktask(ktask, "one");
    new_ktask(freeprinter, "freeprinter");
    new_ktask(statprinter, "statprinter");
    new_ktask(mtest1, "mtest1");
    new_ktask(mtest2, "mtest2");
    new_ktask(mtest3, "mtest3");
    new_ktask(templates_tester, "templates_tester");
    new_ktask(templates_tester, "templates_tester2");
    new_ktask(stress_tester, "stress_tester");
    VFSGlobals::mounts.add_mount(new MemFs(&VFSGlobals::root));
    new_ktask(vfs_tester, "vfs_tester");

    for (int i = 0; i < saved_modules_size; i++) {
        GlobalTtyManager.all_tty_putstr("Starting ");
        GlobalTtyManager.all_tty_putstr(saved_modules_names[i]);
        GlobalTtyManager.all_tty_putchar('\n');

        Task *utask = new_utask((void (*)()) 0x00020000, saved_modules_names[i]);
        assert(saved_modules_size > 0);
        utask->vma->mmap_phys((void *) 0x00020000, (void *) KERN_V2P(saved_modules_data[i]),
                              max_saved_module_file_size, PAGE_USER | PAGE_RW);
        start_task(utask);
    }

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