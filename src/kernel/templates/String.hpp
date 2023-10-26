#ifndef STRING_H
#define STRING_H

#include <cstdlib>
#include <utility>

#include "kmem.hpp"
#include "serial.hpp"
#include "string.h"

class String {
public:
    String() noexcept {
        data = static_cast<char *>(kmalloc(1 * sizeof(char)));
        data[0] = '\0';
    }

    String(const char *in) noexcept {
        unsigned int curLen = strlen(in);

        data = static_cast<char *>(kmalloc((curLen + 1) * sizeof(char)));
        data[0] = '\0';

        strcat(data, in);
    }

    String(String const &str) noexcept {
        unsigned int curLen = strlen(str.data);

        data = static_cast<char *>(kmalloc((curLen + 1) * sizeof(char)));
        data[0] = '\0';

        strcat(data, str.data);
    }

    String(String &&str) noexcept {
        data = str.data;
        str.data = nullptr;
    }

    String &operator=(String str) noexcept {
        std::swap(data, str.data);
        return *this;
    }

    ~String() noexcept {
        if (data == nullptr) return;
        kfree(data);
        data = nullptr;
    }

    String &operator+=(String const &rhs) {
        data = static_cast<char *>(krealloc(data, sizeof(char) * (strlen(data) + strlen(rhs.data) + 1)));
        assert(data != nullptr);

        strcat(data, rhs.data);

        return *this;
    }

    String &operator+=(int value) {
        char buf[20];
        itoa(value, buf, 10);

        *this += buf;

        return *this;
    }

    const char *c_str() {
        return data;
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
    //        unsigned int curLen = 0;
    char *data;
};

#endif