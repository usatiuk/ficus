//
// Created by Stepan Usatiuk on 18.08.2023.
//

#ifndef OS1_TASK_H
#define OS1_TASK_H

#include <stdbool.h>

#include "idt.hpp"

#define TASK_SS 16384

struct Mutex;
struct CV;

enum TaskMode {
    TASKMODE_KERN,
    TASKMODE_USER
};

enum TaskState {
    TS_RUNNING,
    TS_BLOCKED,
    TS_TO_REMOVE,
    TS_TO_SLEEP
};

struct Task {
    struct task_frame frame;
    struct AddressSpace *addressSpace;
    uint64_t *stack;
    char *name;
    enum TaskMode mode;
    uint64_t sleep_until;
    enum TaskState state;
};

struct Task *cur_task();

void init_tasks();
struct Task *new_ktask(void (*fn)(), char *name);
void remove_self();
void sleep_self(uint64_t diff);
extern "C" void switch_task(struct task_frame *cur_frame);
void switch_task_int(struct task_frame *cur_frame);
void wait_m_on_self(struct Mutex *m);
void m_unlock_sched_hook(struct Mutex *m);
void wait_cv_on_self(struct CV *cv);
void stop_waiting_on(struct Mutex *m);
void yield_self();
extern "C" void _yield_self_kern();// Expects the caller to save interrupt state
void cv_unlock_sched_hook(struct CV *cv, int who);

#endif//OS1_TASK_H
