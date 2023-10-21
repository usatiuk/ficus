//
// Created by Stepan Usatiuk on 20.08.2023.
//

#include "mutex.hpp"
#include "serial.hpp"
#include "task.hpp"
#include "timer.hpp"

void m_init(struct Mutex *m) {
    atomic_init(&m->locked, false);
    m->waiters = NULL;
    m->spin_success = 127;
    m->owner = NULL;
}

bool m_try_lock(struct Mutex *m) {
    bool expected = false;
    if (!m->locked.compare_exchange_strong(expected, true)) {
        return false;
    }
    m->owner = cur_task();
    return true;
}

void m_spin_lock(struct Mutex *m) {
    while (!m_try_lock(m)) { __builtin_ia32_pause(); }
}

void m_lock(struct Mutex *m) {
    bool spin_success = false;

    if (m_try_lock(m)) {
        if (m->spin_success < 255)
            m->spin_success++;
        return;
    }

    if (m->spin_success >= 127) {
        uint64_t startMicros = micros;
        while (micros - startMicros < 10) {

            if (m_try_lock(m)) {
                spin_success = true;
                break;
            }

            __builtin_ia32_pause();
        }
    }

    if (spin_success) {
        if (m->spin_success < 255)
            m->spin_success++;
        return;
    } else {
        if (m->spin_success > 0)
            m->spin_success--;

        while (!m_try_lock(m)) {
            wait_m_on_self(m);
        }
    }
}

void m_unlock(struct Mutex *m) {
    bool expected = true;
    if (!m->locked.compare_exchange_strong(expected, false))
        writestr("Unlocking an unlocked mutex!\n");
    m_unlock_sched_hook(m);
}

bool m_test(struct Mutex *m) {
    return atomic_load(&m->locked);
}