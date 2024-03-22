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
    _owner = cur_task();
    return true;
}

//void Mutex::spin_lock() {
//    while (!Mutex::try_lock()) { yield_self(); }
//}

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

            // TODO: this isn't really a spinlock, but for now we don't have SMP
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

        //        for (int i = 0; i < 100000; i++) {
        //            __builtin_ia32_pause();
        //        }

        while (!Mutex::try_lock()) {
            NO_INT(
                    waiters_lock.spinlock();
                    waiters.emplace_front(extract_running_task_node());
                    self_block(waiters_lock););
        }
    }
}

void Mutex::unlock() {
    bool expected = true;
    _owner        = nullptr;
    if (!locked.compare_exchange_strong(expected, false))
        assert2(false, "Unlocking an unlocked mutex!\n");
    List<Task *>::Node *t = nullptr;
    {
        SpinlockLockNoInt l(waiters_lock);
        if (!waiters.empty()) {
            t = waiters.extract_back();
        }
    }
    if (t) unblock(t);
}

bool Mutex::test() {
    return atomic_load(&locked);
}