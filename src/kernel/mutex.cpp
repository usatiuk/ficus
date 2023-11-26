//
// Created by Stepan Usatiuk on 20.08.2023.
//

#include "mutex.hpp"
#include "LockGuard.hpp"
#include "asserts.hpp"
#include "task.hpp"
#include "timer.hpp"


bool Mutex::try_lock() {
    bool expected = false;
    if (!locked.compare_exchange_strong(expected, true)) {
        return false;
    }
    owner = cur_task();
    return true;
}

void Mutex::spin_lock() {
    while (!Mutex::try_lock()) { yield_self(); }
}

void Mutex::lock() {
    bool spinned = false;

    if (Mutex::try_lock()) {
        if (spin_success < 255)
            spin_success++;
        return;
    }

    if (spin_success >= 127) {
        uint64_t startMicros = micros;
        while (micros - startMicros < 10) {

            if (Mutex::try_lock()) {
                spinned = true;
                break;
            }

            yield_self();
        }
    }

    if (spinned) {
        if (spin_success < 255)
            spin_success++;
        return;
    } else {
        if (spin_success > 0)
            spin_success--;

        while (!Mutex::try_lock()) {
            waiters_lock.lock();
            waiters.emplace_front(extract_running_task_node());
            self_block(waiters_lock);
        }
    }
}

void Mutex::unlock() {
    bool expected = true;
    if (!locked.compare_exchange_strong(expected, false))
        assert2(false, "Unlocking an unlocked mutex!\n");
    List<Task *>::Node *t = nullptr;
    {
        LockGuard l(waiters_lock);
        if (!waiters.empty()) {
            t = waiters.back();
            waiters.pop_back();
        }
    }
    if (t) unblock(t);
}

bool Mutex::test() {
    return atomic_load(&locked);
}