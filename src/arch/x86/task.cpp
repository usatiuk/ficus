//
// Created by Stepan Usatiuk on 18.08.2023.
//

#include "task.hpp"
#include "LockGuard.hpp"
#include "SkipList.hpp"
#include "Spinlock.hpp"
#include "TtyManager.hpp"
#include "VMA.hpp"
#include "asserts.hpp"
#include "cv.hpp"
#include "gdt.hpp"
#include "globals.hpp"
#include "kmem.hpp"
#include "misc.hpp"
#include "mutex.hpp"
#include "paging.hpp"
#include "string.h"
#include "timer.hpp"

#include <stl/queue>
#include <stl/vector>

char temp_fxsave[512] __attribute__((aligned(16)));

void sanity_check_frame(TaskFrame *cur_frame) {
    // TODO: This makes sense to check when entering, but not when switching
    //    assert((((uintptr_t) cur_frame) & 0xFULL) == 0);
    assert2((void *) cur_frame->ip != NULL, "Sanity check");
    assert2((void *) cur_frame->sp != NULL, "Sanity check");
    assert2(cur_frame->guard == IDT_GUARD, "IDT Guard wrong!");
    assert(cur_frame->ss != 0);
    assert(cur_frame->cs != 0);
    assert(cur_frame->sp != 0);
    assert2((cur_frame->ss == Arch::GDT::gdt_data.selector() || (cur_frame->ss == Arch::GDT::gdt_data_user.selector()) | 0x3), "SS wrong!");
    assert2((cur_frame->cs == Arch::GDT::gdt_code.selector() || (cur_frame->ss == Arch::GDT::gdt_code_user.selector()) | 0x3), "CS wrong!");
}

std::atomic<uint64_t>               max_pid = 0;
Mutex                               AllTasks_lock;
SkipList<uint64_t, UniquePtr<Task>> AllTasks;

static List<Task *>::Node          *RunningTask;

static Spinlock                     NextTasks_lock;
static List<Task *>                 NextTasks;

// Task freer
Mutex                      TasksToFree_lock;
CV                         TasksToFree_cv;
List<List<Task *>::Node *> TasksToFree;

// Waiting
Mutex WaitingTasks_mlock;
CV    WaitingTasks_cv;
namespace {
    struct
    {
        bool operator()(List<Task *>::Node *l, List<Task *>::Node *r) const { return l->val->_sleep_until > r->val->_sleep_until; }
    } WaitingTaskComparator;
} // namespace
cgistd::priority_queue<List<Task *>::Node *, cgistd::vector<List<Task *>::Node *>, decltype(WaitingTaskComparator)>
        WaitingTasks(WaitingTaskComparator);

//
static std::atomic<bool> initialized = false;

//
static void remove_self() {
    assert(RunningTask != nullptr);
    {
        LockGuard l(TasksToFree_lock);
        // TasksToFree is expected to do nothing with TS_RUNNING tasks
        TasksToFree.emplace_front(RunningTask);
    }
    // This might not cause freeing of this task, as it might be preempted
    // and still be running and task freer won't delete it
    // But eventually it will get cleaned
    TasksToFree_cv.notify_one();

    Scheduler::self_block();
    assert2(0, "should be removed!");
}


static void trampoline(void *rdi, void (*rsi_entrypoint)()) {
    rsi_entrypoint();
    remove_self();
}

Task::Task(Task::TaskMode mode, void (*entrypoint)(), const char *name) {
    _name      = name;

    _frame.ip  = reinterpret_cast<uint64_t>(&trampoline);
    _frame.rsi = (uint64_t) entrypoint;

    if (mode == TaskMode::TASKMODE_KERN) {
        _frame.cs = Arch::GDT::gdt_code.selector();
        _frame.ss = Arch::GDT::gdt_data.selector();
    } else if (mode == TaskMode::TASKMODE_USER) {
        _frame.cs = Arch::GDT::gdt_code_user.selector() | 0x3;
        _frame.ss = Arch::GDT::gdt_data_user.selector() | 0x3;
    } else {
        assert(false);
    }

    for (int i = 0; i < 512; i++) _fxsave->_fxsave[i] = 0;

    _frame.flags = flags();
    _frame.guard = IDT_GUARD;
    if (mode == TaskMode::TASKMODE_USER) {
        _ownAddressSpace = UniquePtr(new AddressSpace());
        _vma             = UniquePtr<VMA>(new VMA(_ownAddressSpace.get()));
    }
    _addressSpace = mode == TaskMode::TASKMODE_KERN ? KERN_AddressSpace : _ownAddressSpace.get();
    _state        = TaskState::TS_BLOCKED;
    _mode         = mode;
    _pid          = max_pid.fetch_add(1);
    _used_time    = 0;

    if (mode == TaskMode::TASKMODE_USER) {
        task_pointer *taskptr = static_cast<task_pointer *>(
                _vma->mmap_mem(reinterpret_cast<void *>(TASK_POINTER),
                               sizeof(task_pointer), 0, PAGE_RW | PAGE_USER)); // FIXME: this is probably unsafe
        assert((uintptr_t) taskptr == TASK_POINTER);

        task_pointer *taskptr_real = reinterpret_cast<task_pointer *>(HHDM_P2V(_addressSpace->virt2real(taskptr)));

        _entry_ksp_val             = ((((uintptr_t) _kstack->_ptr) + (TASK_SS - 9) - 1) & (~0xFULL)); // Ensure 16byte alignment
        // It should be aligned before call, therefore it actually should be aligned here
        assert((_entry_ksp_val & 0xFULL) == 0);

        taskptr_real->taskptr       = this;
        taskptr_real->entry_ksp_val = _entry_ksp_val;
        taskptr_real->ret_sp        = 0x0;
    }

    if (mode == TaskMode::TASKMODE_USER) {
        void *ustack = _vma->mmap_mem(NULL, TASK_SS, 0, PAGE_RW | PAGE_USER);
        _vma->map_kern();

        // Ensure 16byte alignment
        _frame.sp = ((((uintptr_t) ustack) + (TASK_SS - 17) - 1) & (~0xFULL)) + 8;
    } else {
        _frame.sp = ((((uintptr_t) _kstack->_ptr) + (TASK_SS - 9) - 1) & (~0xFULL)) + 8;
    }

    // It should be aligned before call, therefore on function entry it should be misaligned by 8 bytes
    assert((_frame.sp & 0xFULL) == 8);

    sanity_check_frame(&_frame);
    {
        LockGuard l(AllTasks_lock);
        AllTasks.add(_pid, UniquePtr(this));
    }
}

Task::~Task() {
    assert(_state != TaskState::TS_RUNNING);
}

SkipList<uint64_t, std::pair<String, Task::TaskPID>> Scheduler::getTaskTimePerPid() {
    SkipList<uint64_t, std::pair<String, Task::TaskPID>> ret;
    {
        LockGuard l(AllTasks_lock);
        for (const auto &t: AllTasks) {
            ret.add(t.data->pid(), std::make_pair(t.data->name(), t.data->used_time()));
        }
    }
    return ret;
}

static void task_freer() {
    while (true) {
        {
            while (true) {
                {
                    LockGuard l(TasksToFree_lock);
                    if (TasksToFree.empty()) break;
                }
                List<Task *>::Node *t;
                {
                    LockGuard l(TasksToFree_lock);
                    t = TasksToFree.back();
                    if (t->val->state() == Task::TaskState::TS_RUNNING) break;
                    TasksToFree.pop_back();
                }
                {
                    uint64_t pid = t->val->pid();
                    {
                        LockGuard l(AllTasks_lock);
                        AllTasks.erase(pid);
                    }
                }
            }
        }
        {
            // TODO: this is kinda ugly
            TasksToFree_lock.lock();
            TasksToFree_cv.wait(TasksToFree_lock);
            TasksToFree_lock.unlock();
        }
    }
}

void Task::start() {
    assert(_state != TaskState::TS_RUNNING);
    _state        = TaskState::TS_RUNNING;
    auto new_node = NextTasks.create_node(this);
    {
        SpinlockLockNoInt l(NextTasks_lock);
        NextTasks.emplace_front(new_node);
    }
}


void Scheduler::sleep_self(uint64_t diff) {
    uint64_t wake_time = micros + diff;
    while (micros <= wake_time) {
        {
            WaitingTasks_mlock.lock();
            assert(cur_task() != nullptr);
            cur_task()->_sleep_until = wake_time;
            WaitingTasks.push(extract_running_task_node());
            Scheduler::self_block(WaitingTasks_mlock);
        }
    }
}

void Scheduler::yield_self() {
    if (!RunningTask) return;
    NO_INT(
            _yield_self_kern();)
}

static void task_waker() {
    while (true) {
        {
            WaitingTasks_mlock.lock();

            while (!WaitingTasks.empty() && WaitingTasks.top()->val->_sleep_until <= micros) {
                auto task = WaitingTasks.top();
                WaitingTasks.pop();
                WaitingTasks_mlock.unlock();

                task->val->_sleep_until = 0;
                task->val->_state       = Task::TaskState::TS_RUNNING;

                {
                    SpinlockLockNoInt l(NextTasks_lock);
                    NextTasks.emplace_front(task);
                }

                WaitingTasks_mlock.lock();
            }
            WaitingTasks_mlock.unlock();
        }
        {
            // TODO: this is ugly
            WaitingTasks_mlock.lock();
            WaitingTasks_cv.wait(WaitingTasks_mlock);
            WaitingTasks_mlock.unlock();
        }
    }
}

void Scheduler::init_tasks() {
    // FIXME: not actually thread-safe, but it probably doesn't matter
    assert2(!atomic_load(&initialized), "Tasks should be initialized once!");
    (new Task(Task::TaskMode::TASKMODE_KERN, task_freer, "freer"))->start();
    (new Task(Task::TaskMode::TASKMODE_KERN, task_waker, "waker"))->start();
    atomic_store(&initialized, true);
}

extern "C" void Scheduler::switch_task(TaskFrame *cur_frame) {
    assert2(!are_interrupts_enabled(), "Switching tasks with enabled interrupts!");
    if (!atomic_load(&initialized)) return;
    sanity_check_frame(cur_frame);

    assert(!NextTasks_lock.test());

    {
        static uint64_t lastWaitingWakeupMicros = 0;
        if (micros - lastWaitingWakeupMicros > 10000) {
            lastWaitingWakeupMicros = micros;
            WaitingTasks_cv.notify_one();
        }
    }

    AddressSpace       *oldspace = nullptr;
    List<Task *>::Node *next;

    {
        SpinlockLockNoIntAssert ntl(NextTasks_lock);

        static uint64_t         lastSwitchMicros = 0;
        uint64_t                prevSwitchMicros = lastSwitchMicros;
        lastSwitchMicros                         = micros;

        if (RunningTask) {
            RunningTask->val->_frame = *cur_frame;
            __builtin_memcpy(RunningTask->val->_fxsave->_fxsave, temp_fxsave, 512);
            oldspace = RunningTask->val->_addressSpace;
            RunningTask->val->_used_time.fetch_add(lastSwitchMicros - prevSwitchMicros);
            if (RunningTask->val->_state == Task::TaskState::TS_RUNNING) {
                NextTasks.emplace_front(RunningTask);
            }
        }

        next = NextTasks.extract_back();
        assert2(next != NULL, "Kernel left with no tasks!");
        assert2(next->val != NULL, "Kernel left with no tasks!");
        assert2(next->val->_state == Task::TaskState::TS_RUNNING, "Blocked task in run queue!");
    }

    RunningTask = next;
    *cur_frame  = RunningTask->val->_frame;
    __builtin_memcpy(temp_fxsave, RunningTask->val->_fxsave->_fxsave, 512);

    AddressSpace *newspace = RunningTask->val->_addressSpace;

    if (newspace != oldspace) {
        uint64_t real_new_cr3 = (uint64_t) HHDM_V2P(newspace->get_cr3());
        __asm__ volatile("movq %[real_new_cr3], %%cr3"
                         :
                         : [real_new_cr3] "r"(real_new_cr3)
                         : "memory");
    }

    sanity_check_frame(cur_frame);
}

void Scheduler::self_block() {
    // TODO: clarify this function
    NO_INT(
            {
                {
                    SpinlockLockNoInt l(NextTasks_lock);
                    RunningTask->val->_state = Task::TaskState::TS_BLOCKED;
                }
                Scheduler::yield_self();
            })
}

void Scheduler::self_block(Spinlock &to_unlock) {
    {
        SpinlockLockNoInt l(NextTasks_lock);
        to_unlock.unlock();
        RunningTask->val->_state = Task::TaskState::TS_BLOCKED;
    }
    Scheduler::yield_self();
}

void Scheduler::self_block(Mutex &to_unlock) {
    {
        SpinlockLockNoInt l(NextTasks_lock);
        to_unlock.unlock_nolock();
        RunningTask->val->_state = Task::TaskState::TS_BLOCKED;
    }
    Scheduler::yield_self();
}

void Scheduler::unblock(Task *what) {
    assert(false);
    assert(what != nullptr);
    assert(what->_state != Task::TaskState::TS_RUNNING);
    sanity_check_frame(&what->_frame);
    auto new_node = NextTasks.create_node(what);
    {
        SpinlockLockNoInt l(NextTasks_lock);
        assert(what->_state != Task::TaskState::TS_RUNNING);
        what->_state = Task::TaskState::TS_RUNNING;
        NextTasks.emplace_front(new_node);
    }
};

void Scheduler::unblock(List<Task *>::Node *what) {
    assert(what != nullptr);
    assert(what->val->_state != Task::TaskState::TS_RUNNING);
    sanity_check_frame(&what->val->_frame);
    {
        SpinlockLockNoInt l(NextTasks_lock);
        assert(what->val->_state != Task::TaskState::TS_RUNNING);
        what->val->_state = Task::TaskState::TS_RUNNING;
        NextTasks.emplace_front(what);
    }
};
void Scheduler::unblock_nolock(List<Task *>::Node *what) {
    assert(what != nullptr);
    assert(what->val->_state != Task::TaskState::TS_RUNNING);
    sanity_check_frame(&what->val->_frame);
    {
        assert(NextTasks_lock.test() && NextTasks_lock.get_owner() == cur_task());
        assert(what->val->_state != Task::TaskState::TS_RUNNING);
        what->val->_state = Task::TaskState::TS_RUNNING;
        NextTasks.emplace_front(what);
    }
};

Task *Scheduler::cur_task() {
    if (!RunningTask) return NULL;
    return RunningTask->val;
}

List<Task *>::Node *Scheduler::extract_running_task_node() {
    if (!RunningTask) return nullptr;
    return RunningTask;
}