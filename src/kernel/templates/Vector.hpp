#ifndef VECTOR_H
#define VECTOR_H

#include <new>
#include <utility>

#include "assert.h"
#include "kmem.hpp"
#include "string.h"

class VectorTester;

template<typename T>
class Vector {
    friend VectorTester;

public:
    Vector() noexcept {
        _data = static_cast<T *>(kmalloc(_capacity * sizeof(T)));
    }

    explicit Vector(size_t n) noexcept {
        _capacity = n;
        _cur_size = n;
        _data     = static_cast<T *>(kmalloc(_capacity * sizeof(T)));
    }

    Vector(std::initializer_list<T> l) noexcept {
        _cur_size = l.size();
        _capacity = _cur_size > 0 ? _cur_size : 2;

        _data     = static_cast<T *>(kmalloc(_capacity * sizeof(T)));

        size_t i  = 0;
        for (auto const &el: l) {
            new (_data + (i++)) T(el);
        }
    }

    Vector(Vector const &vec) noexcept {
        _cur_size = vec._cur_size;
        _capacity = _cur_size > 0 ? _cur_size : 2;

        _data     = static_cast<T *>(kmalloc(_capacity * sizeof(T)));

        for (size_t i = 0; i < _cur_size; i++)
            new (_data + i) T(vec._data[i]);
    }

    Vector(Vector &&v) noexcept {
        _cur_size = v._cur_size;
        _capacity = v._capacity;
        _data     = v._data;
        v._data   = nullptr;
    }

    Vector &operator=(Vector vec) noexcept {
        std::swap(vec._data, _data);
        std::swap(vec._cur_size, _cur_size);
        std::swap(vec._capacity, _capacity);
        return *this;
    }

    ~Vector() noexcept {
        if (_data == nullptr) return;
        if constexpr (!std::is_trivially_destructible<T>::value)
            for (size_t i = 0; i < _cur_size; i++) _data[i].~T();
        kfree(_data);
        _data = nullptr;
    }

    template<class... Args>
    void emplace_back(Args &&...args) {
        if (_capacity == _cur_size) {
            _capacity *= 2;
            //Ugly hack to get around g++ warnings
            _data = (T *) krealloc(reinterpret_cast<char *>(_data), _capacity * sizeof(T));
            assert(_data != nullptr);
        }
        new (_data + (_cur_size++)) T(std::forward<Args>(args)...);
    }

    void compact() {
        _data     = (T *) krealloc(reinterpret_cast<char *>(_data), _cur_size * sizeof(T));
        _capacity = _cur_size;
    }

    void erase(size_t idx) {
        if constexpr (!std::is_trivially_destructible<T>::value)
            (_data + idx)->~T();
        //Ugly hack to get around g++ warnings
        memmove(reinterpret_cast<char *>(_data + idx), reinterpret_cast<char *>(_data + idx + 1),
                sizeof(T) * (_cur_size - idx - 1));

        _cur_size--;
    }

    T &operator[](size_t idx) {
        return _data[idx];
    }

    T const &operator[](size_t idx) const {
        return _data[idx];
    }

    bool operator==(Vector const &rhs) const {
        if (_cur_size != rhs._cur_size) return false;
        else
            for (size_t i = 0; i < _cur_size; i++)
                if (_data[i] != rhs._data[i]) return false;
        return true;
    }

    bool operator!=(Vector const &rhs) const {
        if (_cur_size != rhs._cur_size) return true;
        else
            for (size_t i = 0; i < _cur_size; i++)
                if (_data[i] != rhs._data[i]) return true;
        return false;
    }

    size_t size() const {
        return _cur_size;
    }

    bool empty() const {
        return _cur_size == 0;
    }
    T &back() {
        assert(size() != 0);
        return _data[size() - 1];
    }

    const T &back() const {
        assert(size() != 0);
        return _data[size() - 1];
    }

    Vector subvector(size_t start, size_t end) const {
        Vector out;
        if (start >= size()) return out;
        end = end > size() ? size() : end;
        for (size_t i = start; i < end; i++) {
            out.emplace_back(_data[i]);
        }
        return out;
    }

    T       *data() { return _data; }
    const T *data() const { return _data; }

    using iterator       = T *;
    using const_iterator = const T *;

    iterator       begin() { return _cur_size > 0 ? _data : end(); }
    iterator       end() { return _data + _cur_size; }
    const_iterator begin() const { return _cur_size > 0 ? _data : end(); }
    const_iterator end() const { return _data + _cur_size; }
    const_iterator cbegin() const { return _cur_size > 0 ? _data : end(); }
    const_iterator cend() const { return _data + _cur_size; }

private:
    size_t _capacity = 2;
    size_t _cur_size = 0;
    T     *_data;
};

#endif