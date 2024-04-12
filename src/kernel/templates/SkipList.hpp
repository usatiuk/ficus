#ifndef SKIPLIST_H
#define SKIPLIST_H

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <new>
#include <type_traits>
#include <utility>

#include "assert.h"
#include "kmem.hpp"

extern "C" int rand(void);

template<typename Data>
class SkipListBase {
protected:
    static constexpr size_t maxL{31};

    class NodeAllocator;
    struct Node {
        friend NodeAllocator;

        Node *next[maxL + 1] = {nullptr};
        Node *before         = nullptr;

        Data &get() {
            assert(!end());
            return *std::launder(reinterpret_cast<Data *>(&_data[0]));
        }
        const Data &get() const {
            assert(!end());
            return *std::launder(reinterpret_cast<const Data *>(&_data[0]));
        }

        bool end() const { return _end; }

    private:
        alignas(Data) std::array<unsigned char, sizeof(Data)> _data;
        bool _end;
    };

    class NodeAllocator {
        static constexpr int size{64};
        Node                *nodes[size];
        int                  top = -1;

        Node                *get() {
            Node *node;
            if (top == -1)
                node = static_cast<Node *>(kmalloc(sizeof(Node)));
            else
                node = nodes[top--];

            node->_end    = false;
            node->before  = nullptr;
            node->next[0] = nullptr;

            return node;
        }

    public:
         NodeAllocator() noexcept = default;

        ~NodeAllocator() noexcept {
            for (int i = top; i >= 0; i--) {
                kfree(nodes[i]);
            }
        }

        NodeAllocator(const NodeAllocator &) = delete;
        // NodeAllocator(NodeAllocator &&)      = delete;
        NodeAllocator &operator=(const NodeAllocator &) = delete;
        // NodeAllocator &operator=(NodeAllocator &&)          = delete;

        //
        void push(Node *&e) {
            if (!e->end()) std::destroy_at(&e->get());
            if (top >= size - 1) {
                kfree(e);
                return;
            }
            nodes[++top] = e;
        }

        template<class... Args>
        Node *get(Args &&...args) {
            Node *ret = get();
            new (&ret->get()) Data(std::forward<Args>(args)...);
            return ret;
        }

        Node *get_end() {
            Node *ret = get();
            ret->_end = true;
            return ret;
        }
    };

    static int randomL() {
        int ret = __builtin_ffs(rand());
        assert(ret >= 0);
        return ret; // NOLINT
    }

    //    static inline NodeAllocator nodeAllocator;
    NodeAllocator nodeAllocator;

    Node         *root;
    Node         *endnode;
    mutable Node *toUpdate[maxL + 1];
    size_t        curL = 0;

                  SkipListBase() noexcept {
        root            = (Node *) nodeAllocator.get_end();
        endnode         = (Node *) nodeAllocator.get_end();
        endnode->before = root;

        for (size_t i = 0; i <= maxL; i++) {
            root->next[i] = endnode;
        }
    };

    // FIXME: Should this be protected?
public:
    ~SkipListBase() noexcept {
        auto cur = root;
        while (cur != nullptr) {
            auto prev = cur;
            cur       = cur->next[0];
            nodeAllocator.push(prev);
        }
    }

    SkipListBase(SkipListBase const &l) noexcept : SkipListBase() {
        toUpdate[0] = root;

        for (auto n = l.root->next[0]; n != nullptr && !n->end(); n = n->next[0]) {
            size_t newLevel = randomL();

            if (newLevel > curL) {
                for (size_t i = curL + 1; i <= newLevel; i++)
                    toUpdate[i] = root;
                curL = newLevel;
            }

            auto newNode    = (Node *) nodeAllocator.get(std::move(n->get()));
            newNode->before = toUpdate[0];
            if (toUpdate[0]->next[0] != nullptr) toUpdate[0]->next[0]->before = newNode;

            for (size_t i = 0; i <= newLevel; i++) {
                newNode->next[i]     = toUpdate[i]->next[i];
                toUpdate[i]->next[i] = newNode;
                toUpdate[i]          = newNode;
            }
        }
    }

    SkipListBase(SkipListBase &&l) noexcept {
        this->root    = l.root;
        l.root        = nullptr;
        this->endnode = l.endnode;
        l.endnode     = nullptr;
        this->curL    = l.curL;
        l.curL        = 0;
    }

    SkipListBase &operator=(SkipListBase l) noexcept {
        std::swap(l.root, root);
        std::swap(l.endnode, endnode);
        std::swap(l.curL, curL);
        return *this;
    }

protected:
    // void add(V *p, size_t n, bool reuseUpdate = false) {
    //     if (!reuseUpdate) {
    //         Node *cur = root;
    //         for (int i = curL; i >= 0; i--) {
    //             while (cur->next[i]->key < p->l && !cur->next[i]->end)
    //                 cur = cur->next[i];
    //             toUpdate[i] = cur;
    //         }
    //     }
    //
    //     for (size_t i = 0; i < n; i++, p++) {
    //         size_t newLevel = randomL();
    //
    //         if (newLevel > curL) {
    //             for (size_t j = curL + 1; j <= newLevel; j++)
    //                 toUpdate[j] = root;
    //
    //             curL = newLevel;
    //         }
    //
    //         auto newNode    = (Node *) nodeAllocator.get();
    //         newNode->key    = p->l;
    //         newNode->data   = *p;
    //
    //         newNode->before = toUpdate[0];
    //         if (toUpdate[0]->next[0] != nullptr) toUpdate[0]->next[0]->before = newNode;
    //
    //         for (size_t j = 0; j <= newLevel; j++) {
    //             newNode->next[j]     = toUpdate[j]->next[j];
    //             toUpdate[j]->next[j] = newNode;
    //             toUpdate[j]          = newNode;
    //         }
    //     }
    // }


    template<typename value_type_arg>
    struct SkipListBaseIteratorBase {
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = value_type_arg;
        using pointer           = value_type *;
        using reference         = value_type &;

        explicit                  SkipListBaseIteratorBase(Node *n) : n(std::move(n)){};

        reference                 operator*() const { return n->get(); }

        pointer                   operator->() const { return &n->get(); }

        SkipListBaseIteratorBase &operator--() {
            if (n->before)
                n = n->before;
            return *this;
        }

        SkipListBaseIteratorBase &operator++() {
            if (n->next[0])
                n = n->next[0];
            return *this;
        }

        SkipListBaseIteratorBase operator++(int) {
            SkipListBaseIteratorBase tmp = *this;
            ++(*this);
            return tmp;
        }

        template<typename L, typename R>
        friend bool operator==(const SkipListBaseIteratorBase<L> &a, const SkipListBaseIteratorBase<R> &b);

        template<typename L, typename R>
        friend bool operator!=(const SkipListBaseIteratorBase<L> &a, const SkipListBaseIteratorBase<R> &b);

        friend SkipListBase;

    protected:
        Node *n;
    };

public:
    template<typename L, typename R>
    friend bool operator==(const SkipListBaseIteratorBase<L> &a, const SkipListBaseIteratorBase<R> &b) {
        if (a.n->end() && b.n->end()) return true;
        return a.n == b.n;
    };

    template<typename L, typename R>
    friend bool operator!=(const SkipListBaseIteratorBase<L> &a, const SkipListBaseIteratorBase<R> &b) {
        if (a.n->end() && (a.n->end() == b.n->end())) return false;
        return a.n != b.n;
    };

    using iterator = SkipListBaseIteratorBase<Data>;

    struct const_iterator : public SkipListBaseIteratorBase<const Data> {
        using SkipListBaseIteratorBase<const Data>::SkipListBaseIteratorBase;
        const_iterator(const iterator &i) : SkipListBaseIteratorBase<const Data>(i.n) {}
    };

protected:
    // Comparator true if less than, 0 if equal
    template<class Comparator>
    std::pair<Node *, bool> erase(const_iterator begin, const_iterator end) {
        if (begin == end) return {root->next[0], false};

        Node *cur = root;
        for (int i = curL; i >= 0; i--) {
            while (!cur->next[i]->end() && Comparator()(cur->next[i]->get(), *begin))
                cur = cur->next[i];
            toUpdate[i] = cur;
        }

        Node *prev = nullptr;
        for (auto cur = begin.n; cur != end.n; cur = cur->next[0]) {
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

        auto ret = std::make_pair(prev->next[0], true);
        if (prev)
            nodeAllocator.push(prev);
        return ret;
    }

    // Comparator true if less than, 0 if equal
    std::pair<Node *, bool> erase(const const_iterator &k) {
        Node *cur = root;

        for (int i = curL; i >= 0; i--) {
            while (!cur->next[i]->end() && cur->next[i] != k.n)
                cur = cur->next[i];
            toUpdate[i] = cur;
        }
        cur = cur->next[0];

        if (cur->end() || cur != k.n) return {cur, false};

        cur->next[0]->before = toUpdate[0];

        for (size_t i = 0; i <= curL; i++) {
            if (toUpdate[i]->next[i] != cur)
                break;

            toUpdate[i]->next[i] = cur->next[i];
        }

        while (curL > 0 &&
               root->next[curL] == nullptr)
            curL--;

        auto ret = std::make_pair(cur->next[0], true);
        nodeAllocator.push(cur);
        return ret;
    };


    // Comparator true if less than, 0 if equal
    template<class Comparator, bool Duplicate>
    std::pair<Node *, bool> insert(Node *newNode) {
        Node *cur = root;

        for (int i = curL; i >= 0; i--) {
            while (!cur->next[i]->end() && Comparator()(cur->next[i]->get(), newNode->get()))
                cur = cur->next[i];
            toUpdate[i] = cur;
        }
        cur = cur->next[0];
        if constexpr (!Duplicate)
            if (!cur->end() && Comparator().eq(cur->get(), newNode->get())) return {cur, false};

        size_t newLevel = randomL();

        if (newLevel > curL) {
            for (size_t i = curL + 1; i <= newLevel; i++)
                toUpdate[i] = root;

            curL = newLevel;
        }

        newNode->before = toUpdate[0];
        if (toUpdate[0]->next[0] != nullptr) toUpdate[0]->next[0]->before = newNode;

        for (size_t i = 0; i <= newLevel; i++) {
            newNode->next[i]     = toUpdate[i]->next[i];
            toUpdate[i]->next[i] = newNode;
            toUpdate[i]          = newNode;
        }
        return {newNode, true};
    }

    template<class Comparator, bool Duplicate>
    std::pair<Node *, bool> insert(Data d) {
        Node *n   = nodeAllocator.get(std::move(d));
        auto  ret = insert<Comparator, Duplicate>(n);
        if (!ret.second) nodeAllocator.push(n);
        return ret;
    }

    template<class Comparator, bool Duplicate, class... Args>
    std::pair<Node *, bool> emplace(Args &&...args) {
        Node *n   = nodeAllocator.get(std::forward<Args>(args)...);
        auto  ret = insert<Comparator, Duplicate>(n);
        if (!ret.second) nodeAllocator.push(n);
        return ret;
    }

    // Comparator true if less than, 0 if equal
    template<class Comparator, typename CmpArg>
    std::pair<Node *, bool> erase(const CmpArg &k) {
        Node *cur = root;

        for (int i = curL; i >= 0; i--) {
            while (!cur->next[i]->end() && Comparator()(cur->next[i]->get(), k))
                cur = cur->next[i];
            toUpdate[i] = cur;
        }
        cur = cur->next[0];

        if (cur->end() || !Comparator().eq(cur->get(), k)) return {cur, false};

        cur->next[0]->before = toUpdate[0];

        for (size_t i = 0; i <= curL; i++) {
            if (toUpdate[i]->next[i] != cur)
                break;

            toUpdate[i]->next[i] = cur->next[i];
        }

        while (curL > 0 &&
               root->next[curL] == nullptr)
            curL--;

        auto ret = std::make_pair(cur->next[0], true);
        nodeAllocator.push(cur);
        return ret;
    };

    template<class Comparator, typename CmpArg>
    Node *upper_bound(const CmpArg &k) const {
        Node *cur = root;

        for (int i = curL; i >= 0; i--)
            while (!cur->next[i]->end() && (Comparator()(cur->next[i]->get(), k) || Comparator().eq(cur->next[i]->get(), k)))
                cur = cur->next[i];
        if (!cur->end() && (Comparator()(cur->get(), k) || Comparator().eq(cur->get(), k)))
            cur = cur->next[0];
        return cur;
    }

    template<class Comparator, typename CmpArg>
    Node *lower_bound(const CmpArg &k) const {
        Node *cur = root;

        for (int i = curL; i >= 0; i--)
            while (!cur->next[i]->end() && Comparator()(cur->next[i]->get(), k))
                cur = cur->next[i];

        return cur->next[0];
    }

public:
    bool operator==(SkipListBase const &r) const {
        auto n  = root->next[0];
        auto n2 = r.root->next[0];

        while (!n->end() && !n2->end()) {
            if (!(n->get() == n2->get())) return false;
            n  = n->next[0];
            n2 = n2->next[0];
        }

        if ((n->end() || n2->end()) && n->end() != n2->end()) return false;

        return true;
    }

    iterator       begin() { return iterator(root->next[0]); }
    iterator       end() { return iterator(endnode); }

    const_iterator begin() const { return const_iterator(root->next[0]); }
    const_iterator end() const { return const_iterator(endnode); }

    const_iterator cbegin() const { return const_iterator(root->next[0]); }
    const_iterator cend() const { return const_iterator(endnode); }

protected:
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


template<bool Duplicates, typename K, typename V, typename CmpBase = std::less<K>>
class SkipListMapBase : public SkipListBase<std::pair<K, V>> {
    using BaseT = SkipListBase<std::pair<K, V>>;

public:
    using iterator       = typename BaseT::iterator;
    using const_iterator = typename BaseT::const_iterator;
    using value_type     = std::pair<K, V>;

private:
    struct Cmp {
        constexpr bool operator()(const value_type &lhs, const K &rhs) const {
            return CmpBase()(lhs.first, rhs);
        }
        constexpr bool operator()(const K &lhs, const value_type &rhs) const {
            return CmpBase()(lhs, rhs.first);
        }
        constexpr bool eq(const value_type &lhs, const K &rhs) const {
            return lhs.first == rhs;
        }
        constexpr bool eq(const K &rhs, const value_type &lhs) const {
            return lhs.first == rhs;
        }
        constexpr bool operator()(const value_type &lhs, const value_type &rhs) const {
            return CmpBase()(lhs.first, rhs.first);
        }
        constexpr bool eq(const value_type &lhs, const value_type &rhs) const {
            return lhs.first == rhs.first;
        }
    };

public:
    iterator       begin() { return BaseT::begin(); }
    iterator       end() { return BaseT::end(); }
    const_iterator begin() const { return BaseT::begin(); }
    const_iterator end() const { return BaseT::end(); }
    const_iterator cbegin() const { return BaseT::cbegin(); }
    const_iterator cend() const { return BaseT::cend(); }

    template<class... Args>
    std::pair<iterator, bool> emplace(Args &&...args) {
        auto r = BaseT::template emplace<Cmp, Duplicates, Args...>(std::forward<Args>(args)...);
        return {iterator(r.first), r.second};
    }

    const_iterator find(const K &k) const {
        typename BaseT::Node *n = BaseT::template lower_bound<Cmp>(k);
        if (n->end() || n->get().first != k) return end();
        return const_iterator(n);
    }

    iterator find(const K &k) {
        typename BaseT::Node *n = BaseT::template lower_bound<Cmp>(k);
        if (n->end() || n->get().first != k) return end();
        return iterator(n);
    }

    const_iterator upper_bound(const K &k) const {
        typename BaseT::Node *n = BaseT::template upper_bound<Cmp>(k);
        if (n->end()) return end();
        return const_iterator(n);
    }

    iterator upper_bound(const K &k) {
        typename BaseT::Node *n = BaseT::template upper_bound<Cmp>(k);
        if (n->end()) return end();
        return iterator(n);
    }

    iterator erase(const K &k) {
        std::pair<typename BaseT::Node *, bool> n = BaseT::template erase<Cmp>(k);
        if (n.first->end()) return end();
        return iterator(n.first);
    }

    iterator erase(const_iterator first, const_iterator last) {
        std::pair<typename BaseT::Node *, bool> n = BaseT::template erase<Cmp>(first, last);
        if (n.first->end()) return end();
        return iterator(n.first);
    }

    iterator erase(const_iterator el) {
        std::pair<typename BaseT::Node *, bool> n = BaseT::erase(el);
        if (n.first->end()) return end();
        return iterator(n.first);
    }
};

template<typename K, typename V, typename CmpBase = std::less<K>>
using SkipListMap = SkipListMapBase<false, K, V, CmpBase>;

//FIXME: erase is probably janky with this
template<typename K, typename V, typename CmpBase = std::less<K>>
using SkipListMultiMap = SkipListMapBase<true, K, V, CmpBase>;

#endif