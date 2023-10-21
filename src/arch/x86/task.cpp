//
// Created by Stepan Usatiuk on 18.08.2023.
//

#include "task.hpp"
#include "LockGuard.hpp"
#include "Spinlock.hpp"
#include "cv.hpp"
#include "gdt.hpp"
#include "kmem.hpp"
#include "misc.hpp"
#include "mutex.hpp"
#include "paging.hpp"
#include "serial.hpp"
#include "timer.hpp"
#include "tty.hpp"

void sanity_check_frame(struct task_frame *cur_frame) {
    assert2((void *) cur_frame->ip != NULL, "Sanity check");
    assert2((void *) cur_frame->sp != NULL, "Sanity check");
    assert2(cur_frame->guard == IDT_GUARD, "IDT Guard wrong!");
    assert2((cur_frame->ss == GDTSEL(gdt_data) || cur_frame->ss == GDTSEL(gdt_data_user)), "SS wrong!");
}

struct TaskListNode {
    struct Task *task;
    struct TaskListNode *next;
};

struct TaskList {
    struct TaskListNode *cur;
    struct TaskListNode *last;
};

struct TaskListNode *RunningTask;

// Should be touched only in the scheduler
struct TaskList NextTasks;

// New tasks
struct Spinlock NewTasks_lock;
struct TaskList NewTasks;

// Unblocked tasks
struct Spinlock UnblockedTasks_lock;
struct TaskList UnblockedTasks;

// Task freer
struct Mutex TasksToFree_lock;
struct CV TasksToFree_cv;
struct TaskList TasksToFree;
struct TaskList TasksToFreeTemp;

// Waiting
//struct Mutex WaitingTasks_lock = DefaultMutex;
struct TaskList WaitingTasks;

static std::atomic<bool> initialized = false;

static void free_task(struct Task *t) {
    kfree(t->stack);
    kfree(t->name);
    kfree(t);
}

static void free_task_list_node(struct TaskListNode *t) {
    kfree(t);
}

static struct TaskListNode *new_task_list_node() {
    struct TaskListNode *ret = static_cast<TaskListNode *>(kmalloc(sizeof(struct TaskListNode)));
    ret->task = NULL;
    ret->next = NULL;
    return ret;
}

static void append_task(struct TaskList *list, struct Task *task) {
    if (list == &NextTasks) {
        assert2(task->state == TS_RUNNING, "Trying to add blocked task to run queue!");
    }

    struct TaskListNode *newNode = new_task_list_node();
    newNode->task = task;

    if (!list->cur) {
        list->cur = newNode;
        list->last = newNode;
    } else {
        list->last->next = newNode;
        list->last = newNode;
    }
}

static void append_task_node(struct TaskList *list, struct TaskListNode *newNode) {
    if (list == &NextTasks) {
        assert2(newNode->task->state == TS_RUNNING, "Trying to add blocked task to run queue!");
    }

    newNode->next = NULL;

    if (!list->cur) {
        assert(list->last == NULL);
        list->cur = newNode;
        list->last = newNode;
    } else {
        list->last->next = newNode;
        list->last = newNode;
    }
}


static struct Task *peek_front(struct TaskList *list) {
    struct Task *ret = NULL;

    if (list->cur) {
        ret = list->cur->task;
    }

    return ret;
}

static struct Task *pop_front(struct TaskList *list) {
    struct Task *ret = NULL;

    if (list->cur) {
        struct TaskListNode *node;
        node = list->cur;
        ret = node->task;
        list->cur = node->next;
        free_task_list_node(node);

        if (list->cur == NULL) list->last = NULL;
    }

    return ret;
}

static struct TaskListNode *pop_front_node(struct TaskList *list) {
    struct TaskListNode *ret = NULL;

    if (list->cur) {
        struct TaskListNode *node;
        node = list->cur;
        ret = node;
        list->cur = node->next;

        if (list->cur == NULL) list->last = NULL;
    } else {
        assert(list->last == NULL);
    }

    if (ret) ret->next = NULL;

    return ret;
}


static void task_freer() {
    while (true) {
        m_lock(&TasksToFree_lock);
        cv_wait(&TasksToFree_lock, &TasksToFree_cv);
        assert2(peek_front(&TasksToFree) != NULL, "Sanity check");
        while (peek_front(&TasksToFree) && peek_front(&TasksToFree)->state == TS_TO_REMOVE) {
            free_task(pop_front(&TasksToFree));
        }
        m_unlock(&TasksToFree_lock);
    }
}

struct Task *new_ktask(void (*fn)(), char *name) {
    struct Task *newt = static_cast<Task *>(kmalloc(sizeof(struct Task)));
    newt->stack = static_cast<uint64_t *>(kmalloc(TASK_SS));
    newt->name = static_cast<char *>(kmalloc(strlen(name) + 1));
    strcpy(name, newt->name);

    newt->frame.sp = ((((uintptr_t) newt->stack) + TASK_SS - 1) & (~0xFULL));// Ensure 16byte alignment
    newt->frame.ip = (uint64_t) fn;
    newt->frame.cs = GDTSEL(gdt_code);
    newt->frame.ss = GDTSEL(gdt_data);
    for (int i = 0; i < 512; i++) newt->frame.ssestate[i] = 0;
    newt->frame.flags = flags();
    newt->frame.guard = IDT_GUARD;
    newt->addressSpace = KERN_AddressSpace;
    newt->state = TS_RUNNING;
    newt->mode = TASKMODE_KERN;

    sanity_check_frame(&newt->frame);

    {
        LockGuard l(NewTasks_lock);
        append_task(&NewTasks, newt);
    }
    return newt;
}

void init_tasks() {
    // FIXME: not actually thread-safe, but it probably doesn't matter
    assert2(!atomic_load(&initialized), "Tasks should be initialized once!");
    new_ktask(task_freer, "freer");
    atomic_store(&initialized, true);
}

void remove_self() {
    RunningTask->task->state = TS_TO_REMOVE;
    yield_self();
    assert2(0, "should be removed!");
}

void sleep_self(uint64_t diff) {
    RunningTask->task->sleep_until = micros + diff;
    RunningTask->task->state = TS_TO_SLEEP;
    yield_self();
}

void yield_self() {
    if (!RunningTask) return;
    NO_INT(
            if (RunningTask->task->mode == TASKMODE_KERN) {
                _yield_self_kern();
            })
}


extern "C" void switch_task(struct task_frame *cur_frame) {
    if (!atomic_load(&initialized)) return;
    sanity_check_frame(cur_frame);

    assert2(!are_interrupts_enabled(), "Switching tasks with enabled interrupts!");

    if (RunningTask) {
        RunningTask->task->frame = *cur_frame;
        if (RunningTask->task->state == TS_RUNNING) {
            assert2(RunningTask->next == NULL, "next should be removed from RunningTask!");
            append_task_node(&NextTasks, RunningTask);
        } else if (RunningTask->task->state == TS_TO_SLEEP) {
            if (!WaitingTasks.cur) {
                assert(WaitingTasks.last == NULL);
                WaitingTasks.cur = RunningTask;
                WaitingTasks.last = RunningTask;
            } else {
                struct TaskListNode *prev = NULL;
                struct TaskListNode *cur = WaitingTasks.cur;

                while (cur && cur->task->sleep_until <= RunningTask->task->sleep_until) {
                    prev = cur;
                    cur = cur->next;
                }

                if (prev) {
                    prev->next = RunningTask;
                    RunningTask->next = cur;
                    if (cur == NULL) WaitingTasks.last = RunningTask;
                } else {
                    RunningTask->next = WaitingTasks.cur;
                    WaitingTasks.cur = RunningTask;
                }

                //        if (cur == WaitingTasks.last) WaitingTasks.last = RunningTask;
            }
        } else if (RunningTask->task->state == TS_TO_REMOVE) {
            append_task_node(&TasksToFreeTemp, RunningTask);
        }
    }

    if (TasksToFreeTemp.cur && !UnblockedTasks_lock.test() && m_try_lock(&TasksToFree_lock)) {
        TasksToFree.cur = TasksToFreeTemp.cur;
        TasksToFree.last = TasksToFreeTemp.last;
        TasksToFreeTemp.cur = NULL;
        TasksToFreeTemp.last = NULL;
        cv_notify_one(&TasksToFree_cv);
        m_unlock(&TasksToFree_lock);
    }

    RunningTask = NULL;

    if (NewTasks_lock.try_lock()) {
        while (peek_front(&NewTasks)) {
            append_task_node(&NextTasks, pop_front_node(&NewTasks));
        }
        NewTasks_lock.unlock();
    }

    if (UnblockedTasks_lock.try_lock()) {
        while (peek_front(&UnblockedTasks)) {
            append_task_node(&NextTasks, pop_front_node(&UnblockedTasks));
        }
        UnblockedTasks_lock.unlock();
    }

    struct TaskListNode *next = pop_front_node(&NextTasks);
    assert2(next != NULL, "Kernel left with no tasks!");
    assert2(next->task != NULL, "Kernel left with no tasks!");
    assert2(next->task->state == TS_RUNNING, "Blocked task in run queue!");

    RunningTask = next;
    *cur_frame = RunningTask->task->frame;

    sanity_check_frame(cur_frame);
}

void switch_task_int(struct task_frame *cur_frame) {
    assert2(!are_interrupts_enabled(), "Switching tasks with enabled interrupts!");
    struct TaskListNode *node = WaitingTasks.cur;

    while (node) {
        if (node->task->sleep_until <= micros && node->task->state == TS_TO_SLEEP) {
            assert2(node->task->sleep_until, "Sleeping until 0?");
            node->task->sleep_until = 0;
            node->task->state = TS_RUNNING;
            append_task_node(&NextTasks, pop_front_node(&WaitingTasks));
            node = WaitingTasks.cur;
        } else {
            break;
        }
    }

    switch_task(cur_frame);
}

void wait_m_on_self(struct Mutex *m) {
    if (!m->waiters) {
        m->waiters = static_cast<TaskList *>(kmalloc(sizeof(struct TaskList)));
        m->waiters->cur = NULL;
        m->waiters->last = NULL;
    }
    // TODO: lock-free?
    NO_INT(append_task_node(m->waiters, RunningTask);
           RunningTask->task->state = TS_BLOCKED;)
    yield_self();
}

void m_unlock_sched_hook(struct Mutex *m) {
    struct TaskListNode *newt = NULL;

    NO_INT(if (m->waiters) {
        newt = pop_front_node(m->waiters);
    })

    if (newt) {
        newt->task->state = TS_RUNNING;
        {
            LockGuard l(UnblockedTasks_lock);
            append_task_node(&UnblockedTasks, newt);
        }
    }
}


void wait_cv_on_self(struct CV *cv) {
    if (!cv->waiters) {
        cv->waiters = static_cast<TaskList *>(kmalloc(sizeof(struct TaskList)));
        cv->waiters->cur = NULL;
        cv->waiters->last = NULL;
    }
    // TODO: lock-free?
    NO_INT(append_task_node(cv->waiters, RunningTask);
           RunningTask->task->state = TS_BLOCKED;)
    yield_self();
}

void cv_unlock_sched_hook(struct CV *cv, int who) {
    struct TaskListNode *newt = NULL;
    do {
        NO_INT(if (cv->waiters) {
            newt = pop_front_node(cv->waiters);
        })

        if (newt) {
            newt->task->state = TS_RUNNING;
            {
                LockGuard l(UnblockedTasks_lock);
                append_task_node(&UnblockedTasks, newt);
            }
        }
    } while (newt && (who == CV_NOTIFY_ALL));
}


struct Task *cur_task() {
    if (!RunningTask) return NULL;
    return RunningTask->task;
}