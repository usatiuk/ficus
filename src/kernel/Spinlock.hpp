//
// Created by Stepan Usatiuk on 21.10.2023.
//

#ifndef OS2_SPINLOCK_H
#define OS2_SPINLOCK_H

#include "serial.hpp"
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
        return true;
    }

    void lock() {
        while (!try_lock()) { yield_self(); }
    }

    void unlock() {
        bool expected = true;
        if (!locked.compare_exchange_strong(expected, false))
            writestr("Unlocking an unlocked spinlock!\n");
    }

    bool test() {
        return locked.load();
    }

private:
    std::atomic<bool> locked = false;
    struct TaskList *waiters = nullptr;
};

static_assert(std::is_trivially_copyable_v<Spinlock> == true);
static_assert(std::is_trivially_destructible_v<Spinlock> == true);

#endif//OS2_SPINLOCK_H
