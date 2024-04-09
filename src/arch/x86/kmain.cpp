//
// Created by Stepan Usatiuk on 13.08.2023.
//
#include <cstddef>

#include "BytesFormatter.hpp"
#include "ElfParser.hpp"
#include "LockGuard.hpp"
#include "MemFs.hpp"
#include "MountTable.hpp"
#include "SerialTty.hpp"
#include "SkipList.hpp"
#include "String.hpp"
#include "TestTemplates.hpp"
#include "TtyManager.hpp"
#include "VFSApi.hpp"
#include "VFSGlobals.hpp"
#include "VFSTester.hpp"
#include "VMA.hpp"
#include "assert.h"
#include "globals.hpp"
#include "kmem.hpp"
#include "limine_fb.hpp"
#include "limine_modules.hpp"
#include "memman.hpp"
#include "misc.h"
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
            Scheduler::sleep_self(25000);
            uint32_t *fb_ptr                               = static_cast<uint32_t *>(framebuffer->address);
            fb_ptr[i * (framebuffer->pitch / 4) + i + 100] = c ? 0 : 0xFFFFFF;
        }
    }
    (new Task(Task::TaskMode::TASKMODE_KERN, ktask, "one"))->start();
}


void ktask() {
    // Ensure we got a framebuffer.
    assert2(framebuffer_count >= 1, "No framebuffer!");

    struct limine_framebuffer *framebuffer = &framebuffers[0];

    for (uint32_t c = 0; c < 2; c++) {
        // Note: we assume the framebuffer model is RGB with 32-bit pixels.
        for (size_t i = 0; i < 100; i++) {
            Scheduler::sleep_self(25000);
            uint32_t *fb_ptr                         = static_cast<uint32_t *>(framebuffer->address);
            fb_ptr[i * (framebuffer->pitch / 4) + i] = c ? 0 : 0xFFFFFF;
        }
    }
    (new Task(Task::TaskMode::TASKMODE_KERN, ktask2, "two"))->start();
}

static Mutex testmutex;

void         mtest1() {
    {
        LockGuard l(testmutex);
        GlobalTtyManager.all_tty_putstr("Locked1\n");
        Scheduler::sleep_self(100000);
    }
    GlobalTtyManager.all_tty_putstr("Unlocked1\n");
}

void mtest2() {
    {
        LockGuard l(testmutex);
        GlobalTtyManager.all_tty_putstr("Locked2\n");
        Scheduler::sleep_self(100000);
    }
    GlobalTtyManager.all_tty_putstr("Unlocked2\n");
}

void mtest3() {
    {
        LockGuard l(testmutex);
        GlobalTtyManager.all_tty_putstr("Locked3\n");
        Scheduler::sleep_self(100000);
    }
    GlobalTtyManager.all_tty_putstr("Unlocked3\n");
}

void stress() {
    static std::atomic<int> i    = 0;
    int                     curi = i++;
    if (curi > 1500) return;

    Scheduler::sleep_self(100000 - curi * 10);

    char buf[69];
    itoa(curi, buf, 10);
    //    GlobalTtyManager.all_tty_putstr("stress ");
    //    GlobalTtyManager.all_tty_putstr(buf);
    //    GlobalTtyManager.all_tty_putstr("\n");
}

void templates_tester() {
    GlobalTtyManager.all_tty_putstr("Testing templates\n");
    for (int i = 0; i < 100; i++)
        test_templates();
    GlobalTtyManager.all_tty_putstr("Testing templates OK\n");
}

void stress_tester() {
    for (int i = 0; i < 100; i++)
        (new Task(Task::TaskMode::TASKMODE_KERN, stress, "stress"))->start();

    GlobalTtyManager.all_tty_putstr("Finished stress\n");
}


void vfs_tester() {
    VFSTester vfsTester;
    vfsTester.test();
}

void ktask_main() {
    GlobalTtyManager.add_tty(new SerialTty());

    (new Task(Task::TaskMode::TASKMODE_KERN, ktask, "one"))->start();
    (new Task(Task::TaskMode::TASKMODE_KERN, mtest1, "mtest1"))->start();
    (new Task(Task::TaskMode::TASKMODE_KERN, mtest2, "mtest2"))->start();
    (new Task(Task::TaskMode::TASKMODE_KERN, mtest3, "mtest3"))->start();
    (new Task(Task::TaskMode::TASKMODE_KERN, templates_tester, "templates_tester"))->start();
    (new Task(Task::TaskMode::TASKMODE_KERN, templates_tester, "templates_tester2"))->start();
    (new Task(Task::TaskMode::TASKMODE_KERN, stress_tester, "stress_tester"))->start();
    VFSGlobals::mounts.add_mount(new MemFs(&VFSGlobals::root));
    (new Task(Task::TaskMode::TASKMODE_KERN, vfs_tester, "vfs_tester"))->start();

    for (int i = 0; i < saved_modules_size; i++) {
        auto &mod = saved_modules[i];

        VFSApi::touch(StrToPath(saved_modules_names[i]));
        FDT::FD fd = VFSApi::open(StrToPath(saved_modules_names[i]));
        File   *f  = VFSApi::get(fd);
        f->write(static_cast<const char *>(mod.address), mod.size);

        if (strcmp(saved_modules_names[i], "/init") == 0) {
            GlobalTtyManager.all_tty_putstr("Starting ");
            GlobalTtyManager.all_tty_putstr(saved_modules_names[i]);
            GlobalTtyManager.all_tty_putchar('\n');

            cgistd::vector<char> read_data(mod.size);
            memcpy(read_data.begin(), mod.address, mod.size);
            ElfParser elfParser(read_data);

            Task     *utask = new Task(Task::TaskMode::TASKMODE_USER, (void (*)()) elfParser.get_entrypoint(), saved_modules_names[i]);
            if (elfParser.copy_to(utask))
                utask->start();
            else
                assert2(false, "Init couldn't be loaded!");
        }

        VFSApi::close(fd);
    }
}

void dummy_task() {
    for (;;) {
        Scheduler::yield_self();
    }
}

extern void (*ctors_begin[])();
extern void (*ctors_end[])();

void        kmain() {
    for (void (**ctor)() = ctors_begin; ctor < ctors_end; ctor++)
        (*ctor)();

    init_timer();

    srand(micros); // NOLINT

    (new Task(Task::TaskMode::TASKMODE_KERN, ktask_main, "ktask_main"))->start();
    (new Task(Task::TaskMode::TASKMODE_KERN, dummy_task, "dummy"))->start();
    setup_syscalls();
    Scheduler::init_tasks();
    for (;;) {
        __asm__ __volatile__("hlt");
    }
}