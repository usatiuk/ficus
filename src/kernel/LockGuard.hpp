//
// Created by Stepan Usatiuk on 21.10.2023.
//

#ifndef FICUS_LOCKGUARD_H
#define FICUS_LOCKGUARD_H

#include "assert.h"

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
        _locked = _lock->try_lock();
    }
    ~LockGuardTry() {
        if (_locked)
            _lock->unlock();
    }

    LockGuardTry(LockGuardTry const &d) = delete;

    bool locked() { return _locked; }
    void lock() {
        _lock->lock();
        _locked = true;
    }

private:
    T   *_lock;
    bool _locked;
};


#endif //FICUS_LOCKGUARD_H
