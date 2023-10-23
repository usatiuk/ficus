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
        owner = cur_task();
        return true;
    }

    void lock() {
        while (!try_lock()) { yield_self(); }
    }

    void unlock() {
        bool expected = true;
        assert(owner == cur_task());
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
    Task *owner;
};

static_assert(std::is_trivially_copyable_v<Spinlock> == true);
static_assert(std::is_trivially_destructible_v<Spinlock> == true);

#endif//OS2_SPINLOCK_H
