//
// Created by Stepan Usatiuk on 18.08.2023.
//

#ifndef OS1_TASK_H
#define OS1_TASK_H

#include "List.hpp"
#include "PointersCollection.hpp"
#include "SkipList.hpp"
#include "String.hpp"
#include "idt.hpp"

#define TASK_SS 16384

class Mutex;
struct AddressSpace;
class VMA;
class Spinlock;

class Task {
public:
    using TaskPID = uint64_t;

    enum class TaskMode {
        TASKMODE_KERN,
        TASKMODE_USER
    };

    enum class TaskState {
        TS_RUNNING,
        TS_BLOCKED
    };

    Task(TaskMode mode, void (*entrypoint)(), const char *name);

    Task(const Task &)                                  = delete;
    Task(Task &&)                                       = delete;
    Task                       &operator=(const Task &) = delete;
    Task                       &operator=(Task &&)      = delete;

    void                        start();

    [[nodiscard]] const String &name() const { return _name; }
    [[nodiscard]] TaskPID       pid() const { return _pid; }
    [[nodiscard]] uint64_t      used_time() const { return _used_time; }
    [[nodiscard]] TaskState     state() const { return _state; }

    ~Task();

    //private:
    struct KernStack {
        uint64_t _ptr[TASK_SS] __attribute__((aligned(16)));
    } __attribute__((aligned(16)));

    struct FxSave {
        uint64_t _fxsave[512] __attribute__((aligned(16)));
    } __attribute__((aligned(16)));

    uint64_t              _entry_ksp_val;
    TaskFrame             _frame;
    TaskPID               _pid;
    std::atomic<uint64_t> _used_time;

    // Note that address space must be destroyed after VMA!
    // as VMA frees what it had allocated there too
    UniquePtr<AddressSpace> _ownAddressSpace;

    AddressSpace           *_addressSpace;
    UniquePtr<VMA>          _vma;
    UniquePtr<KernStack>    _kstack{new KernStack()};
    UniquePtr<FxSave>       _fxsave{new FxSave()};
    String                  _name;
    TaskMode                _mode;
    uint64_t                _sleep_until;
    TaskState               _state;
};


struct task_pointer {
    Task    *taskptr;
    uint64_t entry_ksp_val;
    uint64_t ret_sp;
    uint64_t ret_flags;
} __attribute__((packed));

namespace Scheduler {
    Task               *cur_task();
    List<Task *>::Node *extract_running_task_node();

    void                init_tasks();

    void                sleep_self(uint64_t diff);

    void                remove_self();

    void                self_block();
    void                self_block(Spinlock &to_unlock);
    void                self_block(Mutex &to_unlock);

    void                unblock(Task *what);
    void                unblock(List<Task *>::Node *what);
    void                unblock_nolock(List<Task *>::Node *what);

    extern "C" void     switch_task(TaskFrame *cur_frame);

    // TODO: that's quite inefficient!
    SkipList<uint64_t, std::pair<String, Task::TaskPID>> getTaskTimePerPid();

    void                                                 yield_self();
} // namespace Scheduler

// Expects the caller to save interrupt state
extern "C" void _yield_self_kern();

#endif //OS1_TASK_H
