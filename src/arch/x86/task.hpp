//
// Created by Stepan Usatiuk on 18.08.2023.
//

#ifndef FICUS_TASK_H
#define FICUS_TASK_H

#include "List.hpp"
#include "PointersCollection.hpp"
#include "SkipList.hpp"
#include "String.hpp"
#include "idt.hpp"
#include "task_arch.hpp"

#include <sys/types.h>

#define TASK_SS 65536

class Mutex;
struct AddressSpace;
class VMA;
class Spinlock;

class Task {
public:
    enum class TaskMode {
        TASKMODE_KERN,
        TASKMODE_USER
    };

    enum class TaskState {
        TS_RUNNING,
        TS_BLOCKED,
        TS_ZOMBIE,
        TS_WAITPID_BLOCKED
    };

    /// PID of processor that we're waiting for
    /// or -1, wait for anything, then it's set by the zombified process to its PID
    pid_t _woken_pid = -1;
    /// Place to put list node when waiting for pid/being zombified
    List<Task *>::Node *_waitpid_node;

    Task(TaskMode mode, void (*entrypoint)(), const char *name);

    Task(const Task &)            = delete;
    Task(Task &&)                 = delete;
    Task &operator=(const Task &) = delete;
    Task &operator=(Task &&)      = delete;

    void start();

    [[nodiscard]] const String &name() const { return _name; }
    [[nodiscard]] pid_t         pid() const { return _pid; }
    [[nodiscard]] uint64_t      used_time() const { return _used_time; }
    [[nodiscard]] TaskState     state() const { return _state; }

    ~Task();

    Task *clone();
    void  user_setup();
    void  user_reset();

    //private:
    struct KernStack {
        uint64_t _ptr[TASK_SS] __attribute__((aligned(16)));
    } __attribute__((aligned(16)));

    struct FxSave {
        uint64_t _fxsave[512] __attribute__((aligned(16)));
    } __attribute__((aligned(16)));

    uint64_t              _entry_ksp_val;
    Arch::TaskFrame       _frame;
    pid_t                 _pid;
    std::atomic<uint64_t> _used_time;

    // Note that address space must be destroyed after VMA!
    // as VMA frees what it had allocated there too
    UniquePtr<AddressSpace> _ownAddressSpace;

    AddressSpace        *_addressSpace;
    UniquePtr<VMA>       _vma;
    UniquePtr<KernStack> _kstack{new KernStack()};
    UniquePtr<FxSave>    _fxsave{new FxSave()};
    String               _name;
    TaskMode             _mode;
    uint64_t             _sleep_until;
    TaskState            _state;
    pid_t                _parent = -1;
};


struct task_pointer {
    Task    *taskptr;
    uint64_t entry_ksp_val;
    uint64_t ret_sp;
    uint64_t ret_flags;
    uint64_t ret_ip;
    uint64_t exec_ip;
} __attribute__((packed));

namespace Scheduler {
    Task               *cur_task();
    List<Task *>::Node *extract_running_task_node();

    void init_tasks();

    void sleep_self(uint64_t diff);

    void remove_self();
    void dispose_self();
    void zombify_self();
    void dispose_zombie(Task *zombie);

    void self_block();
    void self_block(Spinlock &to_unlock);
    void self_block(Mutex &to_unlock);

    void unblock(Task *what);
    void unblock(List<Task *>::Node *what);
    void unblock_nolock(List<Task *>::Node *what);

    void  waitpid_block();
    pid_t waitpid(pid_t pid, int *status, int options);

    extern "C" void switch_task(Arch::TaskFrame *cur_frame);

    // TODO: that's quite inefficient!
    SkipListMap<pid_t, std::pair<String, uint64_t>> getTaskTimePerPid();

    void yield_self();
} // namespace Scheduler

// Expects the caller to save interrupt state
extern "C" void _yield_self_kern();

#endif //FICUS_TASK_H
