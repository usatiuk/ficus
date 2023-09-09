//
// Created by Stepan Usatiuk on 18.08.2023.
//

#include "task.h"
#include "cv.h"
#include "gdt.h"
#include "kmem.h"
#include "misc.h"
#include "mutex.h"
#include "paging.h"
#include "serial.h"
#include "timer.h"
#include "tty.h"

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
struct Mutex NewTasks_lock = DefaultMutex;
struct TaskList NewTasks;

// Unblocked tasks
struct Mutex UnblockedTasks_lock = DefaultMutex;
struct TaskList UnblockedTasks;

// Task freer
struct Mutex TasksToFree_lock = DefaultMutex;
struct CV TasksToFree_cv = DefaultCV;
struct TaskList TasksToFree;
struct TaskList TasksToFreeTemp;

// Waiting
//struct Mutex WaitingTasks_lock = DefaultMutex;
struct TaskList WaitingTasks;

static volatile atomic_bool initialized = false;

static void free_task(struct Task *t) {
    kfree(t->stack);
    kfree(t->name);
    kfree(t);
}

static void free_task_list_node(struct TaskListNode *t) {
    kfree(t);
}

static struct TaskListNode *new_task_list_node() {
    struct TaskListNode *ret = kmalloc(sizeof(struct TaskListNode));
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


_Noreturn static void task_freer() {
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

struct Task *new_ktask(void(*fn), char *name) {
    struct Task *new = kmalloc(sizeof(struct Task));
    new->stack = kmalloc(TASK_SS);
    new->name = kmalloc(strlen(name) + 1);
    strcpy(name, new->name);

    new->frame.sp = ((uint64_t) (&((void *) new->stack)[TASK_SS - 1]) & (~0xFULL));// Ensure 16byte alignment
    new->frame.ip = (uint64_t) fn;
    new->frame.cs = GDTSEL(gdt_code);
    new->frame.ss = GDTSEL(gdt_data);
    new->frame.flags = flags();
    new->frame.guard = IDT_GUARD;
    new->addressSpace = KERN_AddressSpace;
    new->state = TS_RUNNING;
    new->mode = TASKMODE_KERN;

    m_lock(&NewTasks_lock);
    append_task(&NewTasks, new);
    m_unlock(&NewTasks_lock);
    return new;
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


void switch_task(struct task_frame *cur_frame) {
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

    if (TasksToFreeTemp.cur && !m_test(&UnblockedTasks_lock) && m_try_lock(&TasksToFree_lock)) {
        TasksToFree.cur = TasksToFreeTemp.cur;
        TasksToFree.last = TasksToFreeTemp.last;
        TasksToFreeTemp.cur = NULL;
        TasksToFreeTemp.last = NULL;
        cv_notify_one(&TasksToFree_cv);
        m_unlock(&TasksToFree_lock);
    }

    RunningTask = NULL;

    if (m_try_lock(&NewTasks_lock)) {
        while (peek_front(&NewTasks)) {
            append_task_node(&NextTasks, pop_front_node(&NewTasks));
        }
        m_unlock(&NewTasks_lock);
    }

    if (m_try_lock(&UnblockedTasks_lock)) {
        while (peek_front(&UnblockedTasks)) {
            append_task_node(&NextTasks, pop_front_node(&UnblockedTasks));
        }
        m_unlock(&UnblockedTasks_lock);
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
    static uint64_t lastSwitchMicros = 0;
    uint64_t curMicros = micros;

    assert2(!are_interrupts_enabled(), "Switching tasks with enabled interrupts!");
    if ((curMicros - lastSwitchMicros) > 1) {
        struct TaskListNode *node = WaitingTasks.cur;

        while (node) {
            if (node->task->sleep_until <= curMicros && node->task->state == TS_TO_SLEEP) {
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
        lastSwitchMicros = curMicros;
    }
}

void wait_m_on_self(struct Mutex *m) {
    if (!m->waiters) {
        m->waiters = kmalloc(sizeof(struct TaskList));
        m->waiters->cur = NULL;
        m->waiters->last = NULL;
    }
    // TODO: lock-free?
    NO_INT(append_task_node(m->waiters, RunningTask);
           RunningTask->task->state = TS_BLOCKED;)
    yield_self();
}

void m_unlock_sched_hook(struct Mutex *m) {
    struct TaskListNode *new = NULL;

    NO_INT(if (m->waiters) {
        new = pop_front_node(m->waiters);
    })

    if (new) {
        new->task->state = TS_RUNNING;
        m_spin_lock(&UnblockedTasks_lock);
        append_task_node(&UnblockedTasks, new);
        m_unlock(&UnblockedTasks_lock);
    }
}


void wait_cv_on_self(struct CV *cv) {
    if (!cv->waiters) {
        cv->waiters = kmalloc(sizeof(struct TaskList));
        cv->waiters->cur = NULL;
        cv->waiters->last = NULL;
    }
    // TODO: lock-free?
    NO_INT(append_task_node(cv->waiters, RunningTask);
           RunningTask->task->state = TS_BLOCKED;)
    yield_self();
}

void cv_unlock_sched_hook(struct CV *cv, int who) {
    struct TaskListNode *new = NULL;
    do {
        NO_INT(if (cv->waiters) {
            new = pop_front_node(cv->waiters);
        })

        if (new) {
            new->task->state = TS_RUNNING;
            m_spin_lock(&UnblockedTasks_lock);
            append_task_node(&UnblockedTasks, new);
            m_unlock(&UnblockedTasks_lock);
        }
    } while (new && (who == CV_NOTIFY_ALL));
}


struct Task *cur_task() {
    if (!RunningTask) return NULL;
    return RunningTask->task;
}
