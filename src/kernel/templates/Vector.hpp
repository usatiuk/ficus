#ifndef VECTOR_H
#define VECTOR_H

#include <new>

#include "asserts.hpp"
#include "kmem.hpp"
#include "string.h"

class VectorTester;

template<typename T>
class Vector {
    friend VectorTester;

public:
    Vector() noexcept {
        data = static_cast<T *>(kmalloc(capacity * sizeof(T)));
    }

    Vector(std::initializer_list<T> l) noexcept {
        curSize = l.size();
        capacity = curSize > 0 ? curSize : 2;

        data = static_cast<T *>(kmalloc(capacity * sizeof(T)));

        size_t i = 0;
        for (auto const &el: l) {
            new (data + (i++)) T(el);
        }
    }

    Vector(Vector const &vec) noexcept {
        curSize = vec.curSize;
        capacity = curSize > 0 ? curSize : 2;

        data = static_cast<T *>(kmalloc(capacity * sizeof(T)));

        for (size_t i = 0; i < curSize; i++)
            new (data + i) T(vec.data[i]);
    }

    Vector(Vector &&v) noexcept {
        curSize = v.curSize;
        capacity = v.capacity;
        data = v.data;
        v.data = nullptr;
    }

    Vector &operator=(Vector vec) noexcept {
        std::swap(vec.data, data);
        std::swap(vec.curSize, curSize);
        std::swap(vec.capacity, capacity);
        return *this;
    }

    ~Vector() noexcept {
        if (data == nullptr) return;
        if constexpr (!std::is_trivially_destructible<T>::value)
            for (size_t i = 0; i < curSize; i++) data[i].~T();
        kfree(data);
        data = nullptr;
    }

    template<class... Args>
    void emplace_back(Args &&...args) {
        if (capacity == curSize) {
            capacity *= 2;
            //Ugly hack to get around g++ warnings
            data = (T *) krealloc(reinterpret_cast<char *>(data), capacity * sizeof(T));
            assert(data != nullptr);
        }
        new (data + (curSize++)) T(std::forward<Args>(args)...);
    }

    void compact() {
        data = (T *) krealloc(reinterpret_cast<char *>(data), curSize * sizeof(T));
        capacity = curSize;
    }

    void erase(size_t idx) {
        if constexpr (!std::is_trivially_destructible<T>::value)
            (data + idx)->~T();
        //Ugly hack to get around g++ warnings
        memmove(reinterpret_cast<char *>(data + idx), reinterpret_cast<char *>(data + idx + 1),
                sizeof(T) * (curSize - idx - 1));

        curSize--;
    }

    T &operator[](size_t idx) {
        return data[idx];
    }

    T const &operator[](size_t idx) const {
        return data[idx];
    }

    bool operator==(Vector const &rhs) const {
        if (curSize != rhs.curSize) return false;
        else
            for (size_t i = 0; i < curSize; i++)
                if (data[i] != rhs.data[i]) return false;
        return true;
    }

    bool operator!=(Vector const &rhs) const {
        if (curSize != rhs.curSize) return true;
        else
            for (size_t i = 0; i < curSize; i++)
                if (data[i] != rhs.data[i]) return true;
        return false;
    }

    size_t size() const {
        return curSize;
    }

    bool empty() const {
        return curSize == 0;
    }

private:
    size_t capacity = 2;
    size_t curSize = 0;
    T *data;
};

#endif