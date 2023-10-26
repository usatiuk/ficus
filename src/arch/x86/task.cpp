//
// Created by Stepan Usatiuk on 18.08.2023.
//

#include "task.hpp"
#include "LockGuard.hpp"
#include "SkipList.hpp"
#include "Spinlock.hpp"
#include "gdt.hpp"
#include "kmem.hpp"
#include "misc.hpp"
#include "mutex.hpp"
#include "paging.hpp"
#include "serial.hpp"
#include "string.h"
#include "timer.hpp"
#include "tty.hpp"

char temp_fxsave[512] __attribute__((aligned(16)));

void sanity_check_frame(struct task_frame *cur_frame) {
    assert((((uintptr_t) cur_frame) & 0xFULL) == 0);
    assert2((void *) cur_frame->ip != NULL, "Sanity check");
    assert2((void *) cur_frame->sp != NULL, "Sanity check");
    assert2(cur_frame->guard == IDT_GUARD, "IDT Guard wrong!");
    assert2((cur_frame->ss == GDTSEL(gdt_data) || cur_frame->ss == GDTSEL(gdt_data_user)), "SS wrong!");
}

std::atomic<uint64_t> max_pid = 0;
Spinlock AllTasks_lock;
SkipList<uint64_t, Task *> AllTasks;

List<Task *>::Node *RunningTask;

Spinlock NextTasks_lock;
List<Task *> NextTasks;

// Task freer
Spinlock TasksToFree_lock;
List<List<Task *>::Node *> TasksToFree;

// Waiting
Spinlock WaitingTasks_lock;
SkipList<uint64_t, List<Task *>::Node *> WaitingTasks;

static std::atomic<bool> initialized = false;

static void free_task(struct Task *t) {
    kfree(t->stack);
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
        sleep_self(10000);
        {
            LockGuard l(TasksToFree_lock);
            while (!TasksToFree.empty()) {
                List<Task *>::Node *t = TasksToFree.back();
                TasksToFree.pop_back();
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
}

struct Task *new_ktask(void (*fn)(), const char *name) {
    struct Task *newt = static_cast<Task *>(kmalloc(sizeof(struct Task)));
    newt->stack = static_cast<uint64_t *>(kmalloc(TASK_SS));
    newt->name = static_cast<char *>(kmalloc(strlen(name) + 1));
    newt->fxsave = static_cast<char *>(kmalloc(512));
    strcpy(name, newt->name);

    newt->frame.sp = ((((uintptr_t) newt->stack) + (TASK_SS - 9) - 1) & (~0xFULL)) + 8;// Ensure 16byte alignment
    // It should be aligned before call, therefore on function entry it should be misaligned by 8 bytes
    assert((newt->frame.sp & 0xFULL) == 8);

    newt->frame.ip = (uint64_t) fn;
    newt->frame.cs = GDTSEL(gdt_code);
    newt->frame.ss = GDTSEL(gdt_data);

    for (int i = 0; i < 512; i++) newt->fxsave[i] = 0;

    newt->frame.flags = flags();
    newt->frame.guard = IDT_GUARD;
    newt->addressSpace = KERN_AddressSpace;
    newt->state = TS_RUNNING;
    newt->mode = TASKMODE_KERN;
    newt->pid = max_pid.fetch_add(1);
    newt->used_time = 0;

    sanity_check_frame(&newt->frame);

    auto new_node = NextTasks.create_node(newt);

    {
        LockGuard l(NextTasks_lock);
        NextTasks.emplace_front(new_node);
    }

    {
        LockGuard l(AllTasks_lock);
        AllTasks.add(newt->pid, newt);
    }
    return newt;
}

void remove_self() {
    {
        LockGuard l(TasksToFree_lock);
        assert(RunningTask != nullptr);
        TasksToFree.emplace_front(RunningTask);
        NextTasks_lock.lock();
        RunningTask->val->state = TS_BLOCKED;
    }
    NextTasks_lock.unlock();
    yield_self();
    assert2(0, "should be removed!");
}

void sleep_self(uint64_t diff) {
    uint64_t wake_time = micros + diff;
    while (micros <= wake_time) {
        {
            LockGuard l(WaitingTasks_lock);

            // TODO this is all ugly
            uint64_t l1 = 0;
            for (auto cur = &*WaitingTasks.begin(); !cur->end; cur = cur->next[0]) l1++;

            assert(RunningTask != nullptr);
            assert(WaitingTasks.add(wake_time, RunningTask) != nullptr);

            uint64_t l2 = 0;
            for (auto cur = &*WaitingTasks.begin(); !cur->end; cur = cur->next[0]) l2++;

            assert(l2 - l1 == 1);
            NextTasks_lock.lock();
            RunningTask->val->state = TS_BLOCKED;
        }
        NextTasks_lock.unlock();
        yield_self();
    }
}

void yield_self() {
    if (!RunningTask) return;
    NO_INT(
            if (RunningTask->val->mode == TASKMODE_KERN) {
                _yield_self_kern();
            })
}

static void task_waker() {
    while (true) {
        {
            LockGuard l(WaitingTasks_lock);

            while (WaitingTasks.begin() != WaitingTasks.end() && WaitingTasks.begin()->key <= micros && WaitingTasks.begin()->data->val->state != TS_RUNNING) {
                auto *node = &*WaitingTasks.begin();
                auto task = WaitingTasks.begin()->data;

                // TODO this is all ugly
                uint64_t l1 = 0;
                for (auto cur = node; !cur->end; cur = cur->next[0]) l1++;

                WaitingTasks.erase(node, node->next[0], false);

                uint64_t l2 = 0;
                for (auto *cur = &*WaitingTasks.begin(); !cur->end; cur = cur->next[0]) l2++;

                assert(l1 - l2 == 1);
                task->val->sleep_until = 0;
                task->val->state = TS_RUNNING;

                {
                    LockGuard l(NextTasks_lock);
                    NextTasks.emplace_front(task);
                }
            }
        }
    }
}

void init_tasks() {
    // FIXME: not actually thread-safe, but it probably doesn't matter
    assert2(!atomic_load(&initialized), "Tasks should be initialized once!");
    new_ktask(task_freer, "freer");
    new_ktask(task_waker, "waker");
    atomic_store(&initialized, true);
}

extern "C" void switch_task(struct task_frame *cur_frame) {
    assert2(!are_interrupts_enabled(), "Switching tasks with enabled interrupts!");
    if (!atomic_load(&initialized)) return;
    sanity_check_frame(cur_frame);

    if (!NextTasks_lock.try_lock()) return;

    static uint64_t lastSwitchMicros = 0;
    uint64_t prevSwitchMicros = lastSwitchMicros;
    lastSwitchMicros = micros;

    if (RunningTask) {
        RunningTask->val->frame = *cur_frame;
        __builtin_memcpy(RunningTask->val->fxsave, temp_fxsave, 512);
        RunningTask->val->used_time.fetch_add(lastSwitchMicros - prevSwitchMicros);
        if (RunningTask->val->state == TS_RUNNING) {
            NextTasks.emplace_front(RunningTask);
        }
    }

    List<Task *>::Node *next = NextTasks.extract_back();
    assert2(next != NULL, "Kernel left with no tasks!");
    assert2(next->val != NULL, "Kernel left with no tasks!");
    assert2(next->val->state == TS_RUNNING, "Blocked task in run queue!");

    NextTasks_lock.unlock();

    RunningTask = next;
    *cur_frame = RunningTask->val->frame;
    __builtin_memcpy(temp_fxsave, RunningTask->val->fxsave, 512);

    sanity_check_frame(cur_frame);
}

void self_block() {
    {
        LockGuard l(NextTasks_lock);
        RunningTask->val->state = TS_BLOCKED;
    }
    yield_self();
}

void self_block(Spinlock &to_unlock) {
    {
        LockGuard l(NextTasks_lock);
        to_unlock.unlock();
        RunningTask->val->state = TS_BLOCKED;
    }
    yield_self();
}

void unblock(Task *what) {
    what->state = TS_RUNNING;
    auto new_node = NextTasks.create_node(what);
    {
        LockGuard l(NextTasks_lock);
        NextTasks.emplace_front(new_node);
    }
};

void unblock(List<Task *>::Node *what) {
    what->val->state = TS_RUNNING;
    {
        LockGuard l(NextTasks_lock);
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