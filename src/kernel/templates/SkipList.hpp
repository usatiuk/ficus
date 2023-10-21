#ifndef SKIPLIST_H
#define SKIPLIST_H

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

#include "rand.h"
#include "serial.hpp"

template<typename K, typename V>
class SkipList {
    static constexpr size_t maxL{31};

public:
    struct Node {
        Node *next[maxL + 1] = {nullptr};
        Node *before = nullptr;
        bool end = false;
        K key = K();

        V data = V();
    };

private:
    class NodeAllocator {
        static constexpr int size{64};
        Node *nodes[size];
        int top = -1;

    public:
        NodeAllocator() noexcept = default;

        ~NodeAllocator() noexcept {
            for (int i = top; i >= 0; i--) {
                //                delete nodes[i];
            }
        }

        void push(Node *&e) {
            if (top >= size - 1) {
                // TODO: ????
                //                delete e;
                return;
            }
            nodes[++top] = e;
        }


        Node *get() {
            if (top == -1) {
                return new Node;
            }

            Node *node = nodes[top--];

            node->end = false;
            node->before = nullptr;
            node->next[0] = nullptr;
            node->key = K();
            //                node->data = V();

            return node;
        }
    };

    static int randomL() {
        int ret = __builtin_ffs(rand());
        assert(ret >= 0);
        return ret;// NOLINT
    }

    //    static inline NodeAllocator nodeAllocator;
    NodeAllocator nodeAllocator;

    Node *root;
    Node *endnode;
    mutable Node *toUpdate[maxL + 1];
    size_t curL = 0;

public:
    SkipList() noexcept {
        root = (Node *) nodeAllocator.get();
        root->end = true;
        endnode = (Node *) nodeAllocator.get();
        endnode->end = true;
        endnode->before = root;

        for (size_t i = 0; i <= maxL; i++) {
            root->next[i] = endnode;
        }
    };

    ~SkipList() noexcept {
        auto cur = root;
        while (cur != nullptr) {
            auto prev = cur;
            cur = cur->next[0];
            nodeAllocator.push(prev);
        }
    }

    SkipList(SkipList const &l) noexcept : SkipList() {
        toUpdate[0] = root;

        for (auto n = l.root->next[0]; n != nullptr && !n->end; n = n->next[0]) {
            size_t newLevel = randomL();

            if (newLevel > curL) {
                for (size_t i = curL + 1; i <= newLevel; i++)
                    toUpdate[i] = root;
                curL = newLevel;
            }

            auto newNode = (Node *) nodeAllocator.get();
            newNode->key = n->key;
            newNode->data = n->data;
            newNode->before = toUpdate[0];
            if (toUpdate[0]->next[0] != nullptr) toUpdate[0]->next[0]->before = newNode;

            for (size_t i = 0; i <= newLevel; i++) {
                newNode->next[i] = toUpdate[i]->next[i];
                toUpdate[i]->next[i] = newNode;
                toUpdate[i] = newNode;
            }
        }
    }

    SkipList(SkipList &&l) noexcept {
        this->root = l.root;
        l.root = nullptr;
        this->endnode = l.endnode;
        l.endnode = nullptr;
        this->curL = l.curL;
        l.curL = 0;
    }

    SkipList &operator=(SkipList l) noexcept {
        std::swap(l.root, root);
        std::swap(l.endnode, endnode);
        std::swap(l.curL, curL);
        return *this;
    }

    void add(V *p, size_t n, bool reuseUpdate) {
        if (!reuseUpdate) {
            Node *cur = root;
            for (int i = curL; i >= 0; i--) {
                while (cur->next[i]->key < p->l && !cur->next[i]->end)
                    cur = cur->next[i];
                toUpdate[i] = cur;
            }
        }

        for (size_t i = 0; i < n; i++, p++) {
            size_t newLevel = randomL();

            if (newLevel > curL) {
                for (size_t j = curL + 1; j <= newLevel; j++)
                    toUpdate[j] = root;

                curL = newLevel;
            }

            auto newNode = (Node *) nodeAllocator.get();
            newNode->key = p->l;
            newNode->data = *p;

            newNode->before = toUpdate[0];
            if (toUpdate[0]->next[0] != nullptr) toUpdate[0]->next[0]->before = newNode;

            for (size_t j = 0; j <= newLevel; j++) {
                newNode->next[j] = toUpdate[j]->next[j];
                toUpdate[j]->next[j] = newNode;
                toUpdate[j] = newNode;
            }
        }
    }

    bool erase(Node *begin, Node *end, bool reuseUpdate) {
        if (begin == end) return false;

        if (!reuseUpdate) {
            Node *cur = root;
            for (int i = curL; i >= 0; i--) {
                while (cur->next[i]->key < begin->key && !cur->next[i]->end)
                    cur = cur->next[i];
                toUpdate[i] = cur;
            }
        }

        Node *prev = nullptr;
        for (auto cur = begin; cur != end; cur = cur->next[0]) {
            if (prev)
                nodeAllocator.push(prev);

            cur->next[0]->before = toUpdate[0];

            for (size_t i = 0; i <= curL; i++) {
                if (toUpdate[i]->next[i] != cur)
                    break;

                toUpdate[i]->next[i] = cur->next[i];
            }

            while (curL > 0 &&
                   root->next[curL] == nullptr)
                curL--;
            prev = cur;
        }

        if (prev)
            nodeAllocator.push(prev);
        return true;
    }

    Node *add(K const &k, V const &v, bool reuseUpdate) {
        if (!reuseUpdate) {
            Node *cur = root;

            for (int i = curL; i >= 0; i--) {
                while (cur->next[i]->key < k && !cur->next[i]->end)
                    cur = cur->next[i];
                toUpdate[i] = cur;
            }
            cur = cur->next[0];

            if (cur->key == k && !cur->end) return nullptr;
        }

        size_t newLevel = randomL();

        if (newLevel > curL) {
            for (size_t i = curL + 1; i <= newLevel; i++)
                toUpdate[i] = root;

            curL = newLevel;
        }

        auto newNode = (Node *) nodeAllocator.get();
        newNode->key = k;
        newNode->data = v;

        newNode->before = toUpdate[0];
        if (toUpdate[0]->next[0] != nullptr) toUpdate[0]->next[0]->before = newNode;

        for (size_t i = 0; i <= newLevel; i++) {
            newNode->next[i] = toUpdate[i]->next[i];
            toUpdate[i]->next[i] = newNode;
            toUpdate[i] = newNode;
        }
        return newNode;
    }

    bool erase(K const &k) {
        Node *cur = root;

        for (int i = curL; i >= 0; i--) {
            while (cur->next[i]->key < k && !cur->next[i]->end)
                cur = cur->next[i];
            toUpdate[i] = cur;
        }
        cur = cur->next[0];

        if (cur->end || cur->key != k) return false;

        cur->next[0]->before = toUpdate[0];

        for (size_t i = 0; i <= curL; i++) {
            if (toUpdate[i]->next[i] != cur)
                break;

            toUpdate[i]->next[i] = cur->next[i];
        }

        while (curL > 0 &&
               root->next[curL] == nullptr)
            curL--;
        nodeAllocator.push(cur);
        return true;
    };


    // Returns the node PRECEDING the node with a key that is GREATER than k
    Node *find(K const &k) const {
        Node *cur = root;

        for (int i = curL; i >= 0; i--)
            while (cur->next[i]->key <= k && !cur->next[i]->end)
                cur = cur->next[i];

        return cur;
    }

    Node *upper_bound(K const &k) const {
        Node *cur = root;

        for (int i = curL; i >= 0; i--) {
            while (cur->next[i]->key <= k && !cur->next[i]->end)
                cur = cur->next[i];
        }

        cur = cur->next[0];

        return cur;
    }

    Node *lower_bound_update(K const &k) const {
        Node *cur = root;

        for (int i = curL; i >= 0; i--) {
            while (cur->next[i]->key < k && !cur->next[i]->end)
                cur = cur->next[i];
            toUpdate[i] = cur;
        }

        cur = cur->next[0];

        return cur;
    }


    Node *lower_bound(K const &k) const {
        Node *cur = root;

        for (int i = curL; i >= 0; i--) {
            while (cur->next[i]->key < k && !cur->next[i]->end)
                cur = cur->next[i];
        }

        cur = cur->next[0];

        return cur;
    }


    bool operator==(SkipList const &r) const {
        auto n = root->next[0];
        auto n2 = r.root->next[0];

        while (!n->end && !n2->end) {
            if (!(n->data == n2->data)) return false;
            n = n->next[0];
            n2 = n2->next[0];
        }

        if ((n->end || n2->end) && n->end != n2->end) return false;

        return true;
    }


    struct SkipListIterator {
        //        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = const V;
        using pointer = value_type *;
        using reference = value_type &;

        explicit SkipListIterator(Node *n) : n(std::move(n)){};

        reference operator*() const { return n->data; }

        pointer operator->() const { return &(n->data); }

        SkipListIterator &operator--() {
            if (!n->end)
                n = n->before;
            return *this;
        }

        SkipListIterator &operator++() {
            if (!n->end)
                n = n->next[0];
            return *this;
        }

        SkipListIterator operator++(int) {
            SkipListIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        friend bool operator==(const SkipListIterator &a, const SkipListIterator &b) { return a.n == b.n; };

        friend bool operator!=(const SkipListIterator &a, const SkipListIterator &b) { return a.n != b.n; };

    private:
        Node *n;
    };

    //        using iterator = SkipListIterator;
    using const_iterator = SkipListIterator;

    const_iterator begin() const { return SkipListIterator(root->next[0]); }

    const_iterator end() const { return SkipListIterator(endnode); }

    //    void print() const {
    //        std::cout << "LIST STATUS" << std::endl;
    //
    //        for (size_t i = 0; i <= curL; i++) {
    //            Node *n = root->next[i];
    //            std::cout << "L " << i << ": ";
    //            while (n != nullptr && n->next[0]) {
    //                std::cout << "GUARD: " << n->end << " KEY: " << n->key << " RANGE: " << n->data
    //                          << " <<<<<";
    //                n = n->next[i];
    //            }
    //            std::cout << std::endl;
    //        }
    //    };
};


#endif