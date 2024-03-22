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
        T     val;
        Node *next;
        List *list;
    };

private:
    Node    *head = nullptr;
    Node    *tail = nullptr;

    uint64_t size = 0;

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
        assert(new_node->list == nullptr);
        new_node->list = this;
        if (head) {
            assert(tail != nullptr);
            assert(size > 0);
            head->next = new_node;
            head       = new_node;
        } else {
            assert(size == 0);
            head = new_node;
            tail = new_node;
        }
        size++;
    }

    bool empty() const {
        return tail == nullptr;
    }

    T &back() {
        if (tail != nullptr) {
            assert(size > 0);
            return tail->val;
        }

        assert(false);
    }

    const T &back() const {
        if (tail != nullptr) {
            assert(size > 0);
            return tail->val;
        }

        assert(false);
    }

    void pop_back() {
        if (!head) {
            assert(size == 0);
            return;
        }

        assert(size > 0);
        size--;
        if (tail == head) {
            assert(size == 0);
            delete tail;
            tail = nullptr;
            head = nullptr;
            return;
        }

        auto old_tail = tail;
        tail          = tail->next;
        delete old_tail;
    }

    Node *extract_back() {
        if (!head) {
            assert(size == 0);
            return nullptr;
        }

        assert(size > 0);
        size--;
        if (tail == head) {
            assert(size == 0);
            auto b  = tail;
            tail    = nullptr;
            head    = nullptr;
            b->list = nullptr;
            return b;
        }

        auto old_tail  = tail;
        tail           = tail->next;
        old_tail->list = nullptr;
        return old_tail;
    }

    bool empty() {
        assert(((tail == nullptr) == (head == nullptr) && (head == nullptr) == (size == 0)));
        return head == nullptr;
    }
};


#endif //OS2_LIST_HPP
