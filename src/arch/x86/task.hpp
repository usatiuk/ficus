//
// Created by Stepan Usatiuk on 18.08.2023.
//

#ifndef OS1_TASK_H
#define OS1_TASK_H

#include "idt.hpp"

#define TASK_SS 16384

class Mutex;

enum TaskMode {
    TASKMODE_KERN,
    TASKMODE_USER
};

enum TaskState {
    TS_RUNNING,
    TS_BLOCKED
};

struct Task {
    struct task_frame frame;
    struct AddressSpace *addressSpace;
    uint64_t *stack;
    char *fxsave;
    char *name;
    enum TaskMode mode;
    uint64_t sleep_until;
    enum TaskState state;
};

struct Task *cur_task();

void init_tasks();
struct Task *new_ktask(void (*fn)(), const char *name);
void remove_self();
void sleep_self(uint64_t diff);


void self_block();

class Spinlock;
void self_block(Spinlock &to_unlock);
void unblock(Task *what);

extern "C" void switch_task(struct task_frame *cur_frame);

void yield_self();

extern "C" void _yield_self_kern();// Expects the caller to save interrupt state

#endif//OS1_TASK_H
