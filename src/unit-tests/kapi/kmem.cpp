//
// Created by Stepan Usatiuk on 12.04.2024.
//

#include "kmem.hpp"

#include <cassert>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

#include <execinfo.h>

std::map<uintptr_t, std::pair<size_t, std::string>> track;

// Based on: https://www.gnu.org/software/libc/manual/html_node/Backtraces.html
static std::string get_stacktrace() {
    std::vector<void *> functions(50);
    char              **strings;
    int                 n;

    n       = backtrace(functions.data(), 50);
    strings = backtrace_symbols(functions.data(), n);

    std::stringstream out;

    if (strings != nullptr) {
        out << "Stacktrace:" << std::endl;
        for (int i = 0; i < n; i++) out << strings[i] << std::endl;
    }

    free(strings);
    return out.str();
}

void *kmalloc(size_t n) {
    void *ret = malloc(n);
    track.emplace((uintptr_t) ret, std::make_pair(n, get_stacktrace()));
    return ret;
}
void kfree(void *addr) {
    if (addr == nullptr) return;
    assert(track.contains((uintptr_t) addr));
    track.erase((uintptr_t) addr);
    free(addr);
}
void *krealloc(void *addr, size_t newsize) {
    if (addr != nullptr) {
        assert(track.contains((uintptr_t) addr));
        track.erase((uintptr_t) addr);
    }
    void *ret = realloc(addr, newsize);
    track.emplace((uintptr_t) ret, std::make_pair(newsize, get_stacktrace()));
    return ret;
}

__attribute__((destructor)) void check() {
    for (const auto &t: track) {
        std::cerr << "Leaked " << t.second.first << " bytes" << '\n';
        std::cerr << t.second.second << std::endl;
    }
    assert(track.empty());
}