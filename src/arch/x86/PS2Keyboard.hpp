//
// Created by Stepan Usatiuk on 28.04.2024.
//

#ifndef PS2KEYBOARD_HPP
#define PS2KEYBOARD_HPP

#include <CircularBuffer.hpp>
#include <cv.hpp>
#include <mutex.hpp>


class PS2Keyboard {
    // TODO: Possibly there should be 2 mutexes?
    Mutex       mutex;
    CV          readercv;
    CV          isrcv;
    static void isr(void *tty);

    void                      this_isr();
    void                      this_pooler();
    char                      process_scancode(int);
    bool                      states[256];
    CircularBuffer<char, 512> buf;

public:
         PS2Keyboard();
    char readchar();
};


#endif //PS2KEYBOARD_HPP
