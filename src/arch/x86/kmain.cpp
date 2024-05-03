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
#include "task.hpp"
#include "timer.hpp"

#include <FbTty.hpp>
#include <LimineFramebuffer.hpp>

void templates_tester() {
    // GlobalTtyManager.all_tty_putstr("Testing templates\n");
    for (int i = 0; i < 5; i++)
        test_templates();
    // GlobalTtyManager.all_tty_putstr("Testing templates OK\n");
}

void vfs_tester() {
    VFSTester vfsTester;
    vfsTester.test();
    GlobalTtyManager.all_tty_putstr("Testing vfs OK\n");
}

void ktask_main() {
    for (int i = 0; i < framebuffer_count; i++)
        GlobalTtyManager.add_tty(new FbTty(new LimineFramebuffer(&framebuffers[i])));
    GlobalTtyManager.add_tty(new SerialTty());

    (new Task(Task::TaskMode::TASKMODE_KERN, templates_tester, "templates_tester"))->start();
    (new Task(Task::TaskMode::TASKMODE_KERN, templates_tester, "templates_tester2"))->start();
    VFSGlobals::mounts.add_mount(new MemFs(&VFSGlobals::root));
    (new Task(Task::TaskMode::TASKMODE_KERN, vfs_tester, "vfs_tester"))->start();
    Task *init;
    for (int i = 0; i < saved_modules_size; i++) {
        auto &mod = saved_modules[i];

        VFSApi::touch(StrToPath(saved_modules_names[i]));
        FDT::FD fd = VFSApi::open(StrToPath(saved_modules_names[i]));
        File   *f  = VFSApi::get(fd);
        f->write(static_cast<const char *>(mod.address), mod.size);

        if (strcmp(saved_modules_names[i], "/init") == 0) {
            Vector<char> read_data(mod.size);
            memcpy(read_data.begin(), mod.address, mod.size);
            ElfParser elfParser(std::move(read_data));

            init = new Task(Task::TaskMode::TASKMODE_USER, (void (*)()) elfParser.get_entrypoint(), saved_modules_names[i]);
            if (!elfParser.copy_to(init))
                assert2(false, "Init couldn't be loaded!");
        }

        VFSApi::close(fd);
    }
    GlobalTtyManager.all_tty_putstr("Setup finished \n");
    GlobalTtyManager.all_tty_putstr("Starting init \n");
    init->start();
}

void dummy_task() {
    for (;;) {
        Scheduler::yield_self();
    }
}

extern void (*ctors_begin[])();
extern void (*ctors_end[])();

void kmain() {
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