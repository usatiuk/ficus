//
// Created by Stepan Usatiuk on 21.10.2023.
//

#ifndef OS2_SPINLOCK_H
#define OS2_SPINLOCK_H

#include "asserts.hpp"
#include "task.hpp"
#include <atomic>
#include <cstdint>
#include <type_traits>

class Spinlock {
public:
    bool try_lock() {
        bool expected = false;
        if (!locked.compare_exchange_strong(expected, true)) {
            return false;
        }
        owner = Scheduler::cur_task();
        return true;
    }

    void spinlock() {
        assert2(!are_interrupts_enabled(), "Assuming all spinlocks are without interrupts");
        while (!try_lock()) __builtin_ia32_pause();
    }

    void unlock() {
        bool expected = true;
        assert(owner == Scheduler::cur_task());
        owner = nullptr;
        assert(locked.compare_exchange_strong(expected, false));
        //        if (!locked.compare_exchange_strong(expected, false))
        //            writestr("Unlocking an unlocked spinlock!\n");
    }

    bool test() {
        return locked.load();
    }

    Task *get_owner() { return locked.load() ? owner : nullptr; }

private:
    std::atomic<bool> locked = false;
    Task             *owner;
};

static_assert(std::is_trivially_copyable_v<Spinlock> == true);
static_assert(std::is_trivially_destructible_v<Spinlock> == true);

class SpinlockLockNoIntAssert {
public:
    SpinlockLockNoIntAssert(Spinlock &lock) : lock(&lock) {
        assert2(!are_interrupts_enabled(), "Interrupts are expected to be disabled here!");
        this->lock->spinlock();
    }
    ~SpinlockLockNoIntAssert() {
        lock->unlock();
    }

    SpinlockLockNoIntAssert(SpinlockLockNoIntAssert const &d) = delete;

private:
    Spinlock *lock;
};

class SpinlockLockNoInt {
public:
    SpinlockLockNoInt(Spinlock &lock) : lock(&lock) {
        f = save_irqdisable();
        this->lock->spinlock();
    }
    ~SpinlockLockNoInt() {
        lock->unlock();
        irqrestore(f);
    }

    SpinlockLockNoInt(SpinlockLockNoInt const &d) = delete;

private:
    Spinlock     *lock;
    unsigned long f;
};

#endif //OS2_SPINLOCK_H
