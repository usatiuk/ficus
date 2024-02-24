//
// Created by Stepan Usatiuk on 21.10.2023.
//

#ifndef OS2_LOCKGUARD_H
#define OS2_LOCKGUARD_H

#include "asserts.hpp"

template<typename T>
class LockGuard {
public:
    LockGuard(T &lock) : lock(&lock) {
        assert2(are_interrupts_enabled(), "Trying to lock with disabled interrupts!");
        this->lock->lock();
    }
    ~LockGuard() {
        lock->unlock();
    }

    LockGuard(LockGuard const &d) = delete;

private:
    T *lock;
};
template<typename T>
class LockGuardTry {
public:
    LockGuardTry(T &lock) : _lock(&lock) {
        assert2(are_interrupts_enabled(), "Trying to lock with disabled interrupts!");
        suc = _lock->try_lock();
    }
    ~LockGuardTry() {
        if (suc)
            _lock->unlock();
    }

    LockGuardTry(LockGuardTry const &d) = delete;

    bool locked() { return suc; }
    void lock() {
        _lock->lock();
        suc = true;
    }

private:
    T *_lock;
    bool suc;
};


#endif//OS2_LOCKGUARD_H
