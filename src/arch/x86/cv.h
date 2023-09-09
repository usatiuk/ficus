//
// Created by Stepan Usatiuk on 20.08.2023.
//

#ifndef OS1_CV_H
#define OS1_CV_H

#include <stdatomic.h>
#include <stddef.h>

#if !(ATOMIC_INT_LOCK_FREE == 2)
#error Atomic int isnt lock free!
#endif

struct Mutex;

enum CV_NOTIFY {
    CV_NOTIFY_NONE = 0,
    CV_NOTIFY_ONE = 1,
    CV_NOTIFY_ALL = 2,
};

struct CV {
    atomic_int_fast8_t notified;
    struct TaskList *waiters;
};

static const struct CV DefaultCV = {
        .notified = ATOMIC_VAR_INIT(CV_NOTIFY_NONE),
        .waiters = NULL};

void cv_wait(struct Mutex *m, struct CV *cv);
void cv_notify_one(struct CV *cv);
void cv_notify_all(struct CV *cv);

#endif//OS1_CV_H
