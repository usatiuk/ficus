//
// Created by Stepan Usatiuk on 26.11.2023.
//

#ifndef FICUS_CIRCULARBUFFER_HPP
#define FICUS_CIRCULARBUFFER_HPP

#include "asserts.hpp"

// FIXME
template<typename T, auto S>
class CircularBuffer {
    T   data[S];
    int front, back;

public:
    CircularBuffer() {
        front = -1;
        back  = -1;
    }

    bool full() {
        if (front == 0 && back == S - 1) {
            return true;
        }
        if (front == back + 1) {
            return true;
        }
        return false;
    }

    bool empty() {
        if (front == -1)
            return true;
        else
            return false;
    }

    void push_back(T what) {
        if (full()) {
            assert(false);
        } else {
            if (front == -1) front = 0;
            back       = (back + 1) % S;
            data[back] = what;
        }
    }

    T pop_back() {
        if (empty()) {
            assert(false);
        } else {
            T ret = data[front];
            if (front == back) {
                front = -1;
                back  = -1;
            } else {
                front = (front + 1) % S;
            }
            return ret;
        }
    }
};

#endif //FICUS_CIRCULARBUFFER_HPP
