//
// Created by Stepan Usatiuk on 21.10.2023.
//

#ifndef OS2_LIST_HPP
#define OS2_LIST_HPP

#include <atomic>
#include <cstdint>
#include <utility>

#include "asserts.hpp"

template<typename T>
class List {
public:
    struct Node {
        T val;
        Node *next;
    };

private:
    Node *head = nullptr;
    Node *tail = nullptr;

public:
    List() = default;
    ~List() {
        while (!empty()) {
            pop_back();
        }
    }

    template<class... Args>
    void emplace_front(Args &&...args) {
        emplace_front(new Node{std::forward<Args>(args)..., nullptr});
    }

    template<class... Args>
    Node *create_node(Args &&...args) {
        return new Node{std::forward<Args>(args)..., nullptr};
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

    Node *extract_back() {
        if (!head) return nullptr;

        if (tail == head) {
            auto b = tail;
            tail = nullptr;
            head = nullptr;
            return b;
        }

        auto old_tail = tail;
        tail = tail->next;
        return old_tail;
    }

    bool empty() {
        assert((tail == nullptr) == (head == nullptr));
        return head == nullptr;
    }
};


#endif//OS2_LIST_HPP
