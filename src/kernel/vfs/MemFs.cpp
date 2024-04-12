//
// Created by Stepan Usatiuk on 23.02.2024.
//

#include "MemFs.hpp"
#include "LockGuard.hpp"

#include <algorithm>

Vector<SharedPtr<Node>> MemFs::MemFsNodeDir::children() {
    LockGuard               l(_lock);

    Vector<SharedPtr<Node>> out;
    for (auto c: _children) {
        out.emplace_back(c.second);
    }
    return out;
}

SharedPtr<NodeDir> MemFs::MemFsNodeDir::mkdir(const String &name) {
    LockGuard l(_lock);
    auto      newnode = MemFsNodeDir::create(name);
    _children.emplace(name, static_ptr_cast<Node>(newnode));
    return static_ptr_cast<NodeDir>(newnode);
}
SharedPtr<NodeFile> MemFs::MemFsNodeDir::mkfile(const String &name) {
    LockGuard l(_lock);
    auto      newfile = MemFsNodeFile::create(name);
    _children.emplace(name, static_ptr_cast<Node>(newfile));
    return static_ptr_cast<NodeFile>(newfile);
}
int64_t MemFs::MemFsNodeFile::read(char *buf, size_t start, size_t num) {
    LockGuard l(_lock);
    num = std::min(num, _bytes.size() - start);
    for (size_t i = 0; i < num; i++) {
        buf[i] = _bytes[start + i];
    }
    return num;
}
int64_t MemFs::MemFsNodeFile::write(const char *buf, size_t start, size_t num) {
    LockGuard l(_lock);
    while (_bytes.size() <= start + num) _bytes.emplace_back(0);
    for (size_t i = 0; i < num; i++) {
        _bytes[start + i] = buf[i];
    }
    return num;
}
size_t MemFs::MemFsNodeFile::size() {
    LockGuard l(_lock);
    return _bytes.size();
}
