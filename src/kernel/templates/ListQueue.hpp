//
// Created by Stepan Usatiuk on 21.10.2023.
//

#ifndef OS2_LISTQUEUE_HPP
#define OS2_LISTQUEUE_HPP

#include <atomic>
#include <cstdint>
#include <utility>

#include "serial.hpp"

template<typename T>
class ListQueue {
private:
    struct Node {
        T val;
        Node *next;
    };

    Node *head = nullptr;
    Node *tail = nullptr;

public:
    ListQueue() = default;

    template<class... Args>
    void emplace_front(Args &&...args) {
        emplace_front(new Node{std::forward<Args>(args)..., nullptr});
    }

    void emplace_front(Node *new_node) {
        if (head) {
            assert(tail != nullptr);
            head->next = new_node;
            head = new_node;
        } else {
            head = new_node;
            tail = new_node;
        }
    }

    T &back() {
        if (tail != nullptr) return tail->val;

        assert(false);
    }

    void pop_back() {
        if (!head) return;

        if (tail == head) {
            delete tail;
            tail = nullptr;
            head = nullptr;
            return;
        }

        auto old_tail = tail;
        tail = tail->next;
        delete old_tail;
    }

    bool empty() {
        assert((tail == nullptr) == (head == nullptr));
        return head == nullptr;
    }
};


#endif//OS2_LISTQUEUE_HPP
