//
// Created by Stepan Usatiuk on 20.08.2023.
//

#ifndef OS1_MUTEX_H
#define OS1_MUTEX_H

#include <atomic>
#include <cstddef>
#include <cstdint>


struct Mutex {
    std::atomic<bool> locked;
    struct TaskList *waiters;
    struct Task *owner;
    uint8_t spin_success;
};

void m_init(struct Mutex *m);
void m_lock(struct Mutex *m);
void m_spin_lock(struct Mutex *m);
bool m_try_lock(struct Mutex *m);
void m_unlock(struct Mutex *m);
bool m_test(struct Mutex *m);

#endif//OS1_MUTEX_H
