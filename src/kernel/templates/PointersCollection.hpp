#ifndef POINTERS_H
#define POINTERS_H

#include <utility>

#include "kmem.hpp"

class SharedPtrTester;

template<typename T>
class UniquePtr {
public:
    UniquePtr() = default;

    template<class... Args>
    explicit UniquePtr(Args &&...args) : ptr(new T(std::forward<Args>(args)...)) {}

    explicit UniquePtr(T *data) : ptr(data) {}

    ~UniquePtr() {
        if (ptr == nullptr) return;
        delete ptr;
    }

    UniquePtr(UniquePtr const &other) = delete;
    UniquePtr &operator=(UniquePtr const &other) = delete;

    UniquePtr(UniquePtr &&other) {
        delete ptr;
        ptr = other.ptr;
        other.ptr = nullptr;
    }

    UniquePtr &operator=(UniquePtr &&other) {
        delete ptr;
        ptr = other.ptr;
        other.ptr = nullptr;
        return *this;
    }

    T *operator->() const { return ptr; }

    T &operator*() const { return *ptr; }

    T *get() const noexcept { return ptr; }

    T *release() noexcept {
        auto b = ptr;
        ptr = nullptr;
        return b;
    }

private:
    T *ptr = nullptr;
};

template<typename T>
class SharedPtr {
    friend SharedPtrTester;

public:
    SharedPtr() = default;

    explicit SharedPtr(T *data) : ptr(data), uses(new int(1)) {}

    ~SharedPtr() {
        if (ptr == nullptr || uses == nullptr) return;
        --(*uses);
        if (*uses == 0) {
            delete ptr;
            delete uses;
        }
    }

    SharedPtr(SharedPtr const &other) : ptr(other.ptr), uses(other.uses) {
        ++(*uses);
    }

    SharedPtr(SharedPtr &&other) {
        uses = other.uses;
        ptr = other.ptr;
        other.uses = nullptr;
        other.ptr = nullptr;
    }

    SharedPtr &operator=(SharedPtr other) {
        std::swap(ptr, other.ptr);
        std::swap(uses, other.uses);
        return *this;
    }

    T *operator->() const { return ptr; }

    T &operator*() const { return *ptr; }

    T *get() const noexcept { return ptr; }

    [[nodiscard]] int useCount() const { return *uses; }

private:
    T *ptr = nullptr;
    int *uses = nullptr;
};

class COWTester;

template<typename T>
class COWPointer {
private:
    friend COWTester;
    SharedPtr<T> ptr;

    void copy() {
        if (ptr.get() && ptr.useCount() > 1) {
            ptr = SharedPtr<T>(new T(*ptr));
        }
    }

public:
    COWPointer() = default;

    explicit COWPointer(T *data) : ptr(data) {}

    explicit COWPointer(SharedPtr<T> data) : ptr(std::move(data)) {}

    COWPointer(COWPointer &&other) = default;

    COWPointer(COWPointer const &data) = default;

    COWPointer &operator=(COWPointer other) {
        std::swap(ptr, other.ptr);
        return *this;
    }

    ~COWPointer() = default;

    T *get() const {
        return ptr.get();
    }

    T *getRW() {
        copy();
        return ptr.get();
    }

    int useCount() { return ptr.useCount(); };

    const T &operator*() const {
        return *ptr;
    }

    const T *operator->() const {
        return ptr.operator->();
    }
};


#endif