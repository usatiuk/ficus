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

char temp_fxsave[512] __attribute__((aligned(16)));

void sanity_check_frame(struct task_frame *cur_frame) {
    // TODO: This makes sense to check when entering, but not when switching
    //    assert((((uintptr_t) cur_frame) & 0xFULL) == 0);
    assert2((void *) cur_frame->ip != NULL, "Sanity check");
    assert2((void *) cur_frame->sp != NULL, "Sanity check");
    assert2(cur_frame->guard == IDT_GUARD, "IDT Guard wrong!");
    assert(cur_frame->ss != 0);
    assert(cur_frame->cs != 0);
    assert(cur_frame->sp != 0);
    assert2((cur_frame->ss == GDTSEL(gdt_data) || (cur_frame->ss == GDTSEL(gdt_data_user)) | 0x3), "SS wrong!");
    assert2((cur_frame->cs == GDTSEL(gdt_code) || (cur_frame->ss == GDTSEL(gdt_code_user)) | 0x3), "CS wrong!");
}

std::atomic<uint64_t> max_pid = 0;
Mutex AllTasks_lock;
SkipList<uint64_t, Task *> AllTasks;

static List<Task *>::Node *RunningTask;

static Spinlock NextTasks_lock;
static List<Task *> NextTasks;

// Task freer
Mutex TasksToFree_lock;
CV TasksToFree_cv;
List<List<Task *>::Node *> TasksToFree;

// Waiting
Mutex WaitingTasks_mlock;
SkipList<uint64_t, List<Task *>::Node *> WaitingTasks;

static std::atomic<bool> initialized = false;

static void free_task(struct Task *t) {
    kfree(t->kstack);
    kfree(t->name);
    kfree(t->fxsave);
    kfree(t);
}

SkipList<uint64_t, std::pair<String, uint64_t>> getTaskTimePerPid() {
    SkipList<uint64_t, std::pair<String, uint64_t>> ret;
    {
        LockGuard l(AllTasks_lock);
        for (const auto &t: AllTasks) {
            ret.add(t.data->pid, std::make_pair(t.data->name, t.data->used_time.load()));
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
                    if (t->val->state == TS_RUNNING) break;
                    TasksToFree.pop_back();
                }
                {
                    uint64_t pid = t->val->pid;
                    {
                        LockGuard l(AllTasks_lock);
                        AllTasks.erase(pid);
                    }
                    free_task(t->val);
                    delete t;
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

struct Task *new_ktask(void (*fn)(), const char *name, bool start) {
    struct Task *newt = static_cast<Task *>(kmalloc(sizeof(struct Task)));
    newt->kstack = static_cast<uint64_t *>(kmalloc(TASK_SS));
    newt->name = static_cast<char *>(kmalloc(strlen(name) + 1));
    newt->fxsave = static_cast<char *>(kmalloc(512));
    strcpy(name, newt->name);

    newt->frame.sp = ((((uintptr_t) newt->kstack) + (TASK_SS - 9) - 1) & (~0xFULL)) + 8;// Ensure 16byte alignment
    // It should be aligned before call, therefore on function entry it should be misaligned by 8 bytes
    assert((newt->frame.sp & 0xFULL) == 8);

    newt->frame.ip = (uint64_t) fn;
    newt->frame.cs = GDTSEL(gdt_code);
    newt->frame.ss = GDTSEL(gdt_data);

    for (int i = 0; i < 512; i++) newt->fxsave[i] = 0;

    newt->frame.flags = flags();
    newt->frame.guard = IDT_GUARD;
    newt->addressSpace = KERN_AddressSpace;
    newt->state = start ? TS_RUNNING : TS_BLOCKED;
    newt->mode = TASKMODE_KERN;
    newt->pid = max_pid.fetch_add(1);
    newt->used_time = 0;

    sanity_check_frame(&newt->frame);
    if (start) {
        auto new_node = NextTasks.create_node(newt);

        {
            SpinlockLockNoInt l(NextTasks_lock);
            NextTasks.emplace_front(new_node);
        }
    }

    {
        LockGuard l(AllTasks_lock);
        AllTasks.add(newt->pid, newt);
    }
    return newt;
}
struct Task *new_utask(void (*entrypoint)(), const char *name) {
    Task *newt = static_cast<Task *>(kmalloc(sizeof(struct Task)));
    newt->kstack = static_cast<uint64_t *>(kmalloc(TASK_SS));
    newt->name = static_cast<char *>(kmalloc(strlen(name) + 1));
    newt->fxsave = static_cast<char *>(kmalloc(512));
    strcpy(name, newt->name);

    newt->frame.ip = (uint64_t) entrypoint;
    newt->frame.cs = GDTSEL(gdt_code_user) | 0x3;
    newt->frame.ss = GDTSEL(gdt_data_user) | 0x3;

    for (int i = 0; i < 512; i++) newt->fxsave[i] = 0;

    newt->frame.flags = flags();
    newt->frame.guard = IDT_GUARD;
    newt->addressSpace = new AddressSpace();
    newt->vma = new VMA(newt->addressSpace);
    newt->state = TS_BLOCKED;
    newt->mode = TASKMODE_USER;
    newt->pid = max_pid.fetch_add(1);
    newt->used_time = 0;

    task_pointer *taskptr = static_cast<task_pointer *>(
            newt->vma->mmap_mem(reinterpret_cast<void *>(TASK_POINTER),
                                sizeof(task_pointer), 0, PAGE_RW | PAGE_USER));// FIXME: this is probably unsafe
    assert((uintptr_t) taskptr == TASK_POINTER);

    task_pointer *taskptr_real = reinterpret_cast<task_pointer *>(HHDM_P2V(newt->addressSpace->virt2real(taskptr)));

    newt->entry_ksp_val = ((((uintptr_t) newt->kstack) + (TASK_SS - 9) - 1) & (~0xFULL));// Ensure 16byte alignment
    // It should be aligned before call, therefore it actually should be aligned here
    assert((newt->entry_ksp_val & 0xFULL) == 0);

    taskptr_real->taskptr = newt;
    taskptr_real->entry_ksp_val = newt->entry_ksp_val;
    taskptr_real->ret_sp = 0x0;

    void *ustack = newt->vma->mmap_mem(NULL, TASK_SS, 0, PAGE_RW | PAGE_USER);

    newt->frame.sp = ((((uintptr_t) ustack) + (TASK_SS - 17) - 1) & (~0xFULL)) + 8;// Ensure 16byte alignment
    // It should be aligned before call, therefore on function entry it should be misaligned by 8 bytes
    assert((newt->frame.sp & 0xFULL) == 8);

    newt->vma->map_kern();

    sanity_check_frame(&newt->frame);

    {
        LockGuard l(AllTasks_lock);
        AllTasks.add(newt->pid, newt);
    }
    return newt;
}

List<Task *>::Node *start_task(struct Task *task) {
    assert(task->state != TS_RUNNING);
    task->state = TS_RUNNING;
    auto new_node = NextTasks.create_node(task);
    {
        SpinlockLockNoInt l(NextTasks_lock);
        NextTasks.emplace_front(new_node);
    }
    return new_node;
}


void remove_self() {
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

    self_block();
    assert2(0, "should be removed!");
}

void sleep_self(uint64_t diff) {
    uint64_t wake_time = micros + diff;
    while (micros <= wake_time) {
        {
            LockGuard lm(WaitingTasks_mlock);

            // TODO this is all ugly
            // TODO: also maybe it breaks if it wakes before self_block?
            uint64_t len1 = 0;
            for (auto cur = &*WaitingTasks.begin(); !cur->end; cur = cur->next[0]) len1++;

            assert(RunningTask != nullptr);
            assert(WaitingTasks.add(wake_time, RunningTask) != nullptr);

            uint64_t len2 = 0;
            for (auto cur = &*WaitingTasks.begin(); !cur->end; cur = cur->next[0]) len2++;

            assert(len2 - len1 == 1);
        }
        self_block();
    }
}

void yield_self() {
    if (!RunningTask) return;
    NO_INT(
            _yield_self_kern();)
}

static void task_waker() {
    while (true) {
        {
            WaitingTasks_mlock.lock();

            while (WaitingTasks.begin() != WaitingTasks.end() && WaitingTasks.begin()->key <= micros && WaitingTasks.begin()->data->val->state != TS_RUNNING) {
                auto *node = &*WaitingTasks.begin();
                auto task = WaitingTasks.begin()->data;

                // TODO this is all ugly
                uint64_t l1 = 0;
                for (auto cur = node; !cur->end; cur = cur->next[0]) l1++;

                WaitingTasks.erase(node, node->next[0], false);

                uint64_t l2 = 0;
                for (auto *cur = &*WaitingTasks.begin(); !cur->end; cur = cur->next[0]) l2++;

                WaitingTasks_mlock.unlock();

                assert(l1 - l2 == 1);
                task->val->sleep_until = 0;
                task->val->state = TS_RUNNING;

                {
                    SpinlockLockNoInt l(NextTasks_lock);
                    NextTasks.emplace_front(task);
                }

                WaitingTasks_mlock.lock();
            }
            WaitingTasks_mlock.unlock();
        }
        yield_self();
    }
}

void init_tasks() {
    // FIXME: not actually thread-safe, but it probably doesn't matter
    assert2(!atomic_load(&initialized), "Tasks should be initialized once!");
    start_task(new_ktask(task_freer, "freer", false));
    new_ktask(task_waker, "waker");
    atomic_store(&initialized, true);
}

extern "C" void switch_task(struct task_frame *cur_frame) {
    assert2(!are_interrupts_enabled(), "Switching tasks with enabled interrupts!");
    if (!atomic_load(&initialized)) return;
    sanity_check_frame(cur_frame);

    assert(!NextTasks_lock.test());

    AddressSpace *oldspace = nullptr;
    List<Task *>::Node *next;

    {
        SpinlockLockNoIntAssert ntl(NextTasks_lock);

        static uint64_t lastSwitchMicros = 0;
        uint64_t prevSwitchMicros = lastSwitchMicros;
        lastSwitchMicros = micros;

        if (RunningTask) {
            RunningTask->val->frame = *cur_frame;
            __builtin_memcpy(RunningTask->val->fxsave, temp_fxsave, 512);
            oldspace = RunningTask->val->addressSpace;
            RunningTask->val->used_time.fetch_add(lastSwitchMicros - prevSwitchMicros);
            if (RunningTask->val->state == TS_RUNNING) {
                NextTasks.emplace_front(RunningTask);
            }
        }

        next = NextTasks.extract_back();
        assert2(next != NULL, "Kernel left with no tasks!");
        assert2(next->val != NULL, "Kernel left with no tasks!");
        assert2(next->val->state == TS_RUNNING, "Blocked task in run queue!");
    }

    RunningTask = next;
    *cur_frame = RunningTask->val->frame;
    __builtin_memcpy(temp_fxsave, RunningTask->val->fxsave, 512);

    AddressSpace *newspace = RunningTask->val->addressSpace;

    if (newspace != oldspace) {
        uint64_t real_new_cr3 = (uint64_t) HHDM_V2P(newspace->get_cr3());
        __asm__ volatile("movq %[real_new_cr3], %%cr3"
                         :
                         : [real_new_cr3] "r"(real_new_cr3)
                         : "memory");
    }

    sanity_check_frame(cur_frame);
}

void self_block() {
    // TODO: clarify this function
    NO_INT(
            {
                {
                    SpinlockLockNoInt l(NextTasks_lock);
                    RunningTask->val->state = TS_BLOCKED;
                }
                yield_self();
            })
}

void self_block(Spinlock &to_unlock) {
    assert2(!are_interrupts_enabled(), "Self blocking with enabled interrupts!");

    {
        SpinlockLockNoInt l(NextTasks_lock);
        to_unlock.unlock();
        RunningTask->val->state = TS_BLOCKED;
    }
    yield_self();
}

void unblock(Task *what) {
    assert(false);
    assert(what != nullptr);
    assert(what->state != TS_RUNNING);
    sanity_check_frame(&what->frame);
    auto new_node = NextTasks.create_node(what);
    {
        SpinlockLockNoInt l(NextTasks_lock);
        what->state = TS_RUNNING;
        NextTasks.emplace_front(new_node);
    }
};

void unblock(List<Task *>::Node *what) {
    assert(what != nullptr);
    assert(what->val->state != TS_RUNNING);
    sanity_check_frame(&what->val->frame);
    {
        SpinlockLockNoInt l(NextTasks_lock);
        what->val->state = TS_RUNNING;
        NextTasks.emplace_front(what);
    }
};

struct Task *cur_task() {
    if (!RunningTask) return NULL;
    return RunningTask->val;
}

List<Task *>::Node *extract_running_task_node() {
    if (!RunningTask) return nullptr;
    return RunningTask;
}