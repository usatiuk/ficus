#ifndef STRING_H
#define STRING_H

#include <cstdlib>
#include <utility>

#include "assert.h"
#include "kmem.hpp"
#include "string.h"

// Null terminated string
class String {
public:
    String() {
        //FIXME:
        _data = static_cast<char *>(kmalloc(_cur_len + 1));
    }

    String(const char *in) noexcept {
        _cur_len = strlen(in);
        _data    = static_cast<char *>(kmalloc(_cur_len + 1));
        memcpy(_data, in, _cur_len + 1);
    }

    String(String const &str) noexcept {
        _cur_len = str._cur_len;
        _data    = static_cast<char *>(kmalloc(_cur_len + 1));
        memcpy(_data, str._data, _cur_len + 1);
    }

    String(String &&str) noexcept {
        _data    = str._data;
        _cur_len = str._cur_len;

        str._cur_len = 0;
        str._data    = nullptr;
    }

    String &operator=(String str) noexcept {
        std::swap(_data, str._data);
        std::swap(_cur_len, str._cur_len);
        return *this;
    }

    ~String() noexcept {
        if (_data == nullptr) return;
        kfree(_data);
    }

    String &operator+=(String const &rhs) {
        _data           = static_cast<char *>(krealloc(_data, _cur_len + rhs._cur_len + 1));
        _data[_cur_len] = '\0';
        assert(_data != nullptr);

        strcat(_data, rhs._data);
        _cur_len += rhs._cur_len;

        return *this;
    }

    String &operator+=(unsigned long value) {
        char buf[32];
        itoa(value, buf, 10);

        *this += buf;

        return *this;
    }
    String &operator+=(unsigned long long value) {
        char buf[64];
        itoa(value, buf, 10);

        *this += buf;

        return *this;
    }

    String &operator+=(char c) {
        _data = static_cast<char *>(krealloc(_data, _cur_len + 2));
        assert(_data != nullptr);

        _data[_cur_len]     = c;
        _data[_cur_len + 1] = '\0';
        _cur_len++;
        return *this;
    }

    const char *c_str() const {
        return _data;
    }

    char *data() {
        return _data;
    }

    size_t length() const {
        return _cur_len;
    }

    bool empty() const {
        return _cur_len == 0;
    }

    bool operator==(String const &rhs) const {
        return strcmp(_data, rhs._data) == 0;
    }

    bool operator!=(String const &rhs) const {
        return strcmp(_data, rhs._data) != 0;
    }

    bool operator<(String const &rhs) const {
        return strcmp(_data, rhs._data) < 0;
    }

    bool operator<=(String const &rhs) const {
        return strcmp(_data, rhs._data) <= 0;
    }

    bool operator>(String const &rhs) const {
        return strcmp(_data, rhs._data) > 0;
    }

private:
    size_t _cur_len = 0;
    char  *_data    = nullptr;
};

#endif