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
        data = static_cast<char *>(kmalloc(1 * sizeof(char)));
        curLen = 0;
        data[0] = '\0';
    }

    String(const char *in) noexcept {
        curLen = strlen(in);

        data = static_cast<char *>(kmalloc((curLen + 1) * sizeof(char)));
        data[0] = '\0';

        strcat(data, in);
    }

    String(String const &str) noexcept {
        curLen = str.curLen;

        data = static_cast<char *>(kmalloc((curLen + 1) * sizeof(char)));
        data[0] = '\0';

        strcat(data, str.data);
    }

    String(String &&str) noexcept {
        data = str.data;
        curLen = str.curLen;
        str.data = nullptr;
    }

    String &operator=(String str) noexcept {
        std::swap(data, str.data);
        std::swap(curLen, str.curLen);
        return *this;
    }

    ~String() noexcept {
        if (data == nullptr) return;
        kfree(data);
        data = nullptr;
        curLen = 0;
    }

    String &operator+=(String const &rhs) {
        data = static_cast<char *>(krealloc(data, sizeof(char) * (curLen + rhs.curLen + 1)));
        assert(data != nullptr);

        strcat(data, rhs.data);
        curLen += rhs.curLen;

        return *this;
    }

    String &operator+=(int value) {
        char buf[20];
        itoa(value, buf, 10);

        *this += buf;

        return *this;
    }

    const char *c_str() const {
        return data;
    }

    size_t length() const {
        return curLen;
    }

    bool empty() const {
        return curLen == 0;
    }

    bool operator==(String const &rhs) const {
        return strcmp(data, rhs.data) == 0;
    }

    bool operator!=(String const &rhs) const {
        return strcmp(data, rhs.data) != 0;
    }

    bool operator<(String const &rhs) const {
        return strcmp(data, rhs.data) < 0;
    }

    bool operator<=(String const &rhs) const {
        return strcmp(data, rhs.data) <= 0;
    }

    bool operator>(String const &rhs) const {
        return strcmp(data, rhs.data) > 0;
    }

private:
    size_t curLen = 0;
    char *data;
};

#endif