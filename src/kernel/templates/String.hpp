#ifndef STRING_H
#define STRING_H

#include <cstdlib>
#include <utility>

#include "asserts.hpp"
#include "kmem.hpp"
#include "string.h"

class String {
public:
    String() noexcept {
        _data = static_cast<char *>(kmalloc(1 * sizeof(char)));
        curLen = 0;
        _data[0] = '\0';
    }

    String(const char *in) noexcept {
        curLen = strlen(in);

        _data = static_cast<char *>(kmalloc((curLen + 1) * sizeof(char)));
        _data[0] = '\0';

        strcat(_data, in);
    }

    String(String const &str) noexcept {
        curLen = str.curLen;

        _data = static_cast<char *>(kmalloc((curLen + 1) * sizeof(char)));
        _data[0] = '\0';

        strcat(_data, str._data);
    }

    String(String &&str) noexcept {
        _data = str._data;
        curLen = str.curLen;

        str._data = static_cast<char *>(kmalloc(1 * sizeof(char)));
        str.curLen = 0;
        str._data[0] = '\0';
    }

    String &operator=(String str) noexcept {
        std::swap(_data, str._data);
        std::swap(curLen, str.curLen);
        return *this;
    }

    ~String() noexcept {
        if (_data == nullptr) return;
        kfree(_data);
        _data = nullptr;
        curLen = 0;
    }

    String &operator+=(String const &rhs) {
        _data = static_cast<char *>(krealloc(_data, sizeof(char) * (curLen + rhs.curLen + 1)));
        assert(_data != nullptr);

        strcat(_data, rhs._data);
        curLen += rhs.curLen;

        return *this;
    }

    String &operator+=(unsigned long value) {
        char buf[20];
        itoa(value, buf, 10);

        *this += buf;

        return *this;
    }
    String &operator+=(unsigned long long value) {
        char buf[32];
        itoa(value, buf, 10);

        *this += buf;

        return *this;
    }

    String &operator+=(char c) {
        _data = static_cast<char *>(krealloc(_data, sizeof(char) * (curLen + 2)));
        assert(_data != nullptr);

        _data[curLen] = c;
        _data[curLen + 1] = '\0';
        curLen++;
        return *this;
    }

    const char *c_str() const {
        return _data;
    }
    
    char *data() {
        return _data;
    }

    size_t length() const {
        return curLen;
    }

    bool empty() const {
        return curLen == 0;
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
    size_t curLen = 0;
    char *_data;
};

#endif