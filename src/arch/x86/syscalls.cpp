//
// Created by Stepan Usatiuk on 26.10.2023.
//

#include "syscalls.hpp"
#include "VFSApi.hpp"
#include "VMA.hpp"
#include "syscalls_defs.h"

#include <cstdint>

#include "TtyManager.hpp"
#include "asserts.hpp"
#include "gdt.hpp"
#include "misc.hpp"

#include "BytesFormatter.hpp"
#include "ElfParser.hpp"
#include "FDT.hpp"
#include "File.hpp"
#include "memman.hpp"
#include "paging.hpp"
#include "stl/vector"
#include "task.hpp"
#include "timer.hpp"

// Don't forget the correct order
// Shockingly, it doesn't immediately break and even something simple as putchar works
// even with completely broken 16-bit segments somehow
// But what happens with something more complex is completely bonkers
struct STAR {
    unsigned unused     : 32;
    unsigned call_cs_ss : 16;
    unsigned ret_cs_ss  : 16;
} __attribute__((packed));

static_assert(sizeof(STAR) == 8);

void setup_syscalls() {
    union {
        STAR     star;
        uint64_t bytes;
    } __attribute__((__packed__)) newstar{};

    newstar.star.ret_cs_ss = (Arch::GDT::gdt_data_user.selector() - 8) | 0x3;
    assert(newstar.star.ret_cs_ss + 8 == (Arch::GDT::gdt_data_user.selector() | 0x3));
    assert(newstar.star.ret_cs_ss + 16 == (Arch::GDT::gdt_code_user.selector() | 0x3));

    newstar.star.call_cs_ss = (Arch::GDT::gdt_code.selector());
    assert(newstar.star.call_cs_ss == Arch::GDT::gdt_code.selector());
    assert(newstar.star.call_cs_ss + 8 == Arch::GDT::gdt_data.selector());

    wrmsr(0xc0000081, newstar.bytes);
    wrmsr(0xc0000082, reinterpret_cast<uint64_t>(&_syscall_entrypoint));
    wrmsr(0xc0000084, (1 << 9)); // IA32_FMASK, mask interrupts

    wrmsr(0xC0000080, rdmsr(0xC0000080) | 0b1);
}

void syscall_exit() {
    Scheduler::remove_self();
}

uint64_t syscall_putchar(char c) {
    GlobalTtyManager.all_tty_putchar(c);
    return 0;
}

uint64_t syscall_sleep(uint64_t micros) {
    Scheduler::sleep_self(micros);
    return 0;
}

uint64_t syscall_readchar() {
    Tty *tty = GlobalTtyManager.get_tty(0);
    return tty->readchar();
}

uint64_t syscall_open(const char *pathname, int flags) {
    FDT::FD res = FDT::current()->open(StrToPath(pathname), static_cast<FileOpts>(flags));
    return res;
}

uint64_t syscall_close(uint64_t FD) {
    FDT::current()->close(FD);
    return 1;
}

uint64_t syscall_read(uint64_t fd, char *buf, uint64_t len) {
    auto f = FDT::current()->get(fd);
    if (!f) return -1;
    return f->read(buf, len);
}

uint64_t syscall_write(uint64_t fd, const char *buf, uint64_t len) {
    auto f = FDT::current()->get(fd);
    if (!f) return -1;
    return f->write(buf, len);
}

uint64_t syscall_lseek(uint64_t fd, uint64_t off, uint64_t whence) {
    auto f = FDT::current()->get(fd);
    if (!f) return -1;
    return f->seek(off);
}

uint64_t syscall_print_tasks() {
    static SkipList<uint64_t, std::pair<String, uint64_t>> last_times      = Scheduler::getTaskTimePerPid();
    static std::atomic<uint64_t>                           last_print_time = micros;

    uint64_t                                               prev_print_time = last_print_time;
    last_print_time                                                        = micros;
    SkipList<uint64_t, std::pair<String, uint64_t>> prev_times             = std::move(last_times);
    last_times                                                             = Scheduler::getTaskTimePerPid();

    uint64_t slice                                                         = last_print_time - prev_print_time;
    if (slice == 0) return 0;

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

    return 0;
}

uint64_t syscall_print_mem() {
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

    return 0;
}

uint64_t syscall_execve(const char *pathname, char *const argv[], char *const envp[]) {
    // Just copy for now;
    FDT::FD fd = VFSApi::open(StrToPath(pathname));
    if (fd == -1) return -1;
    File                *f = VFSApi::get(fd);

    cgistd::vector<char> read_data(f->size());
    f->read(read_data.begin(), f->size());
    VFSApi::close(fd);

    ElfParser elfParser(read_data);

    Task     *utask = new Task(Task::TaskMode::TASKMODE_USER, (void (*)()) elfParser.get_entrypoint(), pathname);
    if (elfParser.copy_to(utask))
        utask->start();
    else
        return -1;

    return 0;
}

extern "C" uint64_t syscall_impl(uint64_t id_rdi, uint64_t a1_rsi, uint64_t a2_rdx, uint64_t a3_rcx) {
    assert2(are_interrupts_enabled(), "why wouldn't they be?");
    switch (id_rdi) {
        case SYSCALL_EXIT_ID:
            syscall_exit();
            return 0;
        case SYSCALL_PUTCHAR_ID:
            return syscall_putchar(a1_rsi);
        case SYSCALL_SLEEP_ID:
            return syscall_sleep(a1_rsi);
        case SYSCALL_READCHAR_ID:
            assert(a1_rsi == NULL);
            return syscall_readchar();
        case SYSCALL_OPEN_ID:
            return syscall_open(reinterpret_cast<const char *>(a1_rsi), a2_rdx);
        case SYSCALL_CLOSE_ID:
            return syscall_close(a1_rsi);
        case SYSCALL_READ_ID:
            return syscall_read(a1_rsi, reinterpret_cast<char *>(a2_rdx), a3_rcx);
        case SYSCALL_WRITE_ID:
            return syscall_write(a1_rsi, reinterpret_cast<const char *>(a2_rdx), a3_rcx);
        case SYSCALL_LSEEK_ID:
            return syscall_lseek(a1_rsi, a2_rdx, a3_rcx);
        case SYSCALL_EXECVE_ID:
            return syscall_execve(reinterpret_cast<const char *>(a1_rsi), reinterpret_cast<char *const *>(a2_rdx), reinterpret_cast<char *const *>(a3_rcx));
        case SYSCALL_OPENDIR_ID:
        case SYSCALL_READDIR_ID:
        case SYSCALL_CLOSEDIR_ID:
        case SYSCALL_MKDIR_ID:
        case SYSCALL_UNLINK_ID:
        case SYSCALL_PRINT_TASKS:
            return syscall_print_tasks();
        case SYSCALL_PRINT_MEM:
            return syscall_print_mem();
        default:
            return -1;
    }
}
