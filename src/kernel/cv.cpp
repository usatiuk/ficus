//
// Created by Stepan Usatiuk on 20.08.2023.
//

#include "cv.hpp"

#include "mutex.hpp"
#include "serial.hpp"
#include "task.hpp"

void cv_wait(struct Mutex *m, struct CV *cv) {
    m_unlock(m);
    wait_cv_on_self(cv);
    m_lock(m);
}

void cv_notify_one(struct CV *cv) {
    cv_unlock_sched_hook(cv, CV_NOTIFY_ONE);
}

void cv_notify_all(struct CV *cv) {
    cv_unlock_sched_hook(cv, CV_NOTIFY_ALL);
}
