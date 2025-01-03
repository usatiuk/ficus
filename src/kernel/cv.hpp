//
// Created by Stepan Usatiuk on 26.11.2023.
//

#ifndef FICUS_CV_HPP
#define FICUS_CV_HPP

#include <atomic>
#include <cstddef>
#include <cstdint>

#include "List.hpp"
#include "LockGuard.hpp"
#include "Spinlock.hpp"

struct Task;

// This is probably broken in some way
class CV {
    List<Task *> waiters;
    Spinlock     waiters_lock;

public:
    template<typename Lockable>
    void wait(Lockable &l) {
        NO_INT(
                waiters_lock.spinlock();
                l.unlock();
                // TODO: recheck this is correct
                waiters.emplace_front(Scheduler::extract_running_task_node());
                Scheduler::self_block(waiters_lock);)
        l.lock();
    }
    void notify_one() {
        List<Task *>::Node *t = nullptr;
        {
            SpinlockLockNoInt l(waiters_lock);
            if (!waiters.empty()) {
                t = waiters.extract_back();
            }
        }
        if (t) Scheduler::unblock(t);
    }
    void notify_all() {
        List<Task *> waiters_new;
        {
            SpinlockLockNoInt l(waiters_lock);
            std::swap(waiters_new, waiters);
        }
        while (!waiters_new.empty()) {
            auto t = waiters_new.extract_back();
            Scheduler::unblock(t);
        }
    }
};


#endif //FICUS_CV_HPP
