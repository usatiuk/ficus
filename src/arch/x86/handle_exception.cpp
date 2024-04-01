//
// Created by Stepan Usatiuk on 29.03.2024.
//


#include "task.hpp"

extern "C" __attribute__((noreturn)) void exception_handler_err(uint64_t code) {
    //FIXME:
    if (Scheduler::cur_task()->_mode == Task::TaskMode::TASKMODE_USER) {
        writestr_no_yield("Task ded");
        Scheduler::cur_task()->_state = Task::TaskState::TS_BLOCKED;
        _yield_self_kern();
    } else {
        writestr_no_yield("Kernel ded");
        _hcf();
    }
}
extern "C" __attribute__((noreturn)) void exception_handler_no_err(void) {
    //FIXME:
    if (Scheduler::cur_task()->_mode == Task::TaskMode::TASKMODE_USER) {
        writestr_no_yield("Task ded");
        Scheduler::cur_task()->_state = Task::TaskState::TS_BLOCKED;
        _yield_self_kern();
    } else {
        writestr_no_yield("Kernel ded");
        _hcf();
    }
}
