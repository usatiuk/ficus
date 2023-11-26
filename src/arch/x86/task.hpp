//
// Created by Stepan Usatiuk on 18.08.2023.
//

#ifndef OS1_TASK_H
#define OS1_TASK_H

#include "List.hpp"
#include "SkipList.hpp"
#include "String.hpp"
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

struct AddressSpace;
class VMA;

struct Task {
    uint64_t entry_ksp_val;
    struct task_frame frame;
    uint64_t pid;
    std::atomic<uint64_t> used_time;
    AddressSpace *addressSpace;
    VMA *vma;
    uint64_t *kstack;
    char *fxsave;
    char *name;
    enum TaskMode mode;
    uint64_t sleep_until;
    enum TaskState state;
};

struct task_pointer {
    Task *taskptr;
    uint64_t entry_ksp_val;
    uint64_t ret_sp;
} __attribute__((packed));

struct Task *cur_task();
List<Task *>::Node *extract_running_task_node();

void init_tasks();
struct Task *new_ktask(void (*fn)(), const char *name);
struct Task *new_utask(void (*entrypoint)(), const char *name);
void start_utask(struct Task *task);
void remove_self();
void sleep_self(uint64_t diff);

void self_block();

class Spinlock;
void self_block(Spinlock &to_unlock);
void unblock(Task *what);
void unblock(List<Task *>::Node *what);

extern "C" void switch_task(struct task_frame *cur_frame);

// TODO: that's quite inefficient!
SkipList<uint64_t, std::pair<String, uint64_t>> getTaskTimePerPid();

void yield_self();

extern "C" void _yield_self_kern();// Expects the caller to save interrupt state

#endif//OS1_TASK_H
