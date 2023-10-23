//
// Created by Stepan Usatiuk on 20.08.2023.
//

#ifndef OS1_MUTEX_H
#define OS1_MUTEX_H

#include <atomic>
#include <cstddef>
#include <cstdint>

#include "List.hpp"
#include "Spinlock.hpp"

struct Task;
struct TaskListNode;

class Mutex {
public:
    Mutex() = default;

    void lock();
    void spin_lock();
    bool try_lock();
    void unlock();
    bool test();

private:
    std::atomic<bool> locked = false;

    List<Task *> waiters;
    Spinlock waiters_lock;

    Task *owner = nullptr;
    uint8_t spin_success = 127;
};


#endif//OS1_MUTEX_H
