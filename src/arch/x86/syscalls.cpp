//
// Created by Stepan Usatiuk on 26.10.2023.
//

#include "syscalls.hpp"
#include "VFSApi.hpp"
#include "VMA.hpp"

#include <sys/syscalls.h>
#include <sys/dirent.h>

#include <algorithm>
#include <cstdint>

#include "TtyManager.hpp"
#include "assert.h"
#include "gdt.hpp"
#include "misc.h"

#include "BytesFormatter.hpp"
#include "ElfParser.hpp"
#include "FDT.hpp"
#include "File.hpp"
#include "Vector.hpp"
#include "memman.hpp"
#include "paging.hpp"
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
    FDT::FD res = FDT::current()->open(StrToPath(pathname), flags);
    return res;
}

uint64_t syscall_close(uint64_t FD) {
    FDT::current()->close(FD);
    return 1;
}

//FIXME:
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
    static SkipListMap<uint64_t, std::pair<String, uint64_t>> last_times      = Scheduler::getTaskTimePerPid();
    static std::atomic<uint64_t>                              last_print_time = micros;

    uint64_t                                                  prev_print_time = last_print_time;
    last_print_time                                                           = micros;
    SkipListMap<uint64_t, std::pair<String, uint64_t>> prev_times             = std::move(last_times);
    last_times                                                                = Scheduler::getTaskTimePerPid();

    uint64_t slice                                                            = last_print_time - prev_print_time;
    if (slice == 0) return 0;

    for (const auto &t: prev_times) {
        auto f = last_times.find(t.first);
        if (f != last_times.end()) {
            assert(f->second.second >= t.second.second);
            String buf;
            buf += "PID: ";
            buf += t.first;
            buf += " ";
            buf += t.second.first;
            buf += " usage: ";
            buf += (((f->second.second - t.second.second) * 100ULL) / slice);
            buf += "%\n";
            GlobalTtyManager.all_tty_putstr(buf.c_str());
        } else {
            String buf;
            buf += "PID: ";
            buf += t.first;
            buf += " ";
            buf += t.second.first;
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
    File *f = VFSApi::get(fd);
    if (f->dir().get() != nullptr) return -1;

    Vector<char> read_data(f->size());
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

char *syscall_sbrk(int brk) {
    auto  vma = Scheduler::cur_task()->_vma.get();

    char *ret = reinterpret_cast<char *>(-1);

    if (!vma) return reinterpret_cast<char *>(-1);

    if (!vma->brk_start) {
        vma->brk_start = (char *) vma->mmap_mem(nullptr, VMA::kBrkSize /* 16MB */, 0, PAGE_RW | PAGE_USER);
        if (!vma->brk_start) return reinterpret_cast<char *>(-1); // FIXME:
        vma->brk_end_real = *vma->brk_start + VMA::kBrkSize;
        vma->brk_end_fake = vma->brk_start;
    }

    if (*vma->brk_end_fake + brk >= *vma->brk_start + VMA::kBrkSize) {
        return ret;
    }

    ret               = *vma->brk_end_fake;
    vma->brk_end_fake = *vma->brk_end_fake + brk;

    return ret;
}

int64_t syscall_getdents(int fd, struct dirent *dp, int count) {
    auto f = FDT::current()->get(fd);
    if (!f) return -1;

    auto dir = f->dir();
    if (dir.get() == nullptr) return -1;

    auto children = dir->children();

    count /= sizeof(dirent);

    if (f->pos() >= children.size()) return 0;

    count = std::min(children.size() - f->pos(), (size_t) count);

    for (int i = 0; i < count; i++) {
        auto &child    = children[i + f->pos()];
        dp[i].d_fileno = i + f->pos() + 1;
        strncpy(dp[i].d_name, child->name().c_str(), 255);
        dp[i].d_name[child->name().length() + 1] = '\0';
        dp[i].d_namlen                           = child->name().length();
        dp[i].d_reclen                           = sizeof(dirent);
        dp[i].d_type                             = child->type() == Node::DIR ? 4 : 8;
    }
    f->seek(count + f->pos());
    return count * sizeof(dirent);
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
        case SYSCALL_SBRK_ID:
            return reinterpret_cast<uint64_t>(syscall_sbrk(static_cast<int64_t>(a1_rsi)));
        case SYSCALL_OPENDIR_ID:
            return -1;
        case SYSCALL_READDIR_ID:
            return syscall_getdents(static_cast<int64_t>(a1_rsi), reinterpret_cast<dirent *>(a2_rdx), static_cast<int64_t>(a3_rcx));
        case SYSCALL_CLOSEDIR_ID:
        case SYSCALL_MKDIR_ID:
        case SYSCALL_UNLINK_ID:
            return -1;
        case SYSCALL_PRINT_TASKS:
            return syscall_print_tasks();
        case SYSCALL_PRINT_MEM:
            return syscall_print_mem();
        default:
            return -1;
    }
}
