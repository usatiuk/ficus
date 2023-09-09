//
// Created by Stepan Usatiuk on 20.08.2023.
//

#ifndef OS1_MUTEX_H
#define OS1_MUTEX_H

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if !(ATOMIC_BOOL_LOCK_FREE == 2)
#error Atomic bool isnt lock free!
#endif

struct Mutex {
    volatile atomic_bool locked;
    struct TaskList *waiters;
    struct Task *owner;
    uint8_t spin_success;
};

static const struct Mutex DefaultMutex = {
        .locked = ATOMIC_VAR_INIT(false),
        .spin_success = 150,
        .waiters = NULL};

void m_init(struct Mutex *m);
void m_lock(struct Mutex *m);
void m_spin_lock(struct Mutex *m);
bool m_try_lock(struct Mutex *m);
void m_unlock(struct Mutex *m);
bool m_test(struct Mutex *m);

#endif//OS1_MUTEX_H
