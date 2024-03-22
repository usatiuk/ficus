//
// Created by Stepan Usatiuk on 14.08.2023.
//

#ifndef OS1_TIMER_H
#define OS1_TIMER_H

#include <atomic>
#include <cstdint>

extern volatile std::atomic<uint64_t> ticks;
extern volatile std::atomic<uint64_t> micros;

void                                  init_timer();
void                                  timer_tick();

#endif //OS1_TIMER_H
