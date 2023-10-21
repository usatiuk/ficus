//
// Created by Stepan Usatiuk on 21.10.2023.
//

#ifndef OS2_LOCKGUARD_H
#define OS2_LOCKGUARD_H

template<typename T>
class LockGuard {
public:
    LockGuard(T &lock) : lock(&lock) {
        this->lock->lock();
    }
    ~LockGuard() {
        lock->unlock();
    }

private:
    T *lock;
};


#endif//OS2_LOCKGUARD_H
