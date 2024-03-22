//
// Created by Stepan Usatiuk on 23.02.2024.
//

#include "MemFs.hpp"
#include "LockGuard.hpp"

Vector<Node *> MemFs::MemFsNodeDir::children() {
    LockGuard      l(_lock);

    Vector<Node *> out;
    for (auto c: _children) {
        out.emplace_back(c.data);
    }
    return out;
}

NodeDir *MemFs::MemFsNodeDir::mkdir(const String &name) {
    LockGuard l(_lock);
    auto      newnode = new MemFsNodeDir();
    newnode->_name    = name;
    _children.add(name, newnode);
    return newnode;
}
NodeFile *MemFs::MemFsNodeDir::mkfile(const String &name) {
    LockGuard l(_lock);
    auto      newfile = new MemFsNodeFile(name);
    _children.add(name, newfile);
    return newfile;
}
bool MemFs::MemFsNodeFile::read(char *buf, size_t start, size_t num) {
    LockGuard l(_lock);
    if (start >= _bytes.size()) return false;
    if (start + num > _bytes.size()) return false;
    for (size_t i = 0; i < num; i++) {
        buf[i] = _bytes[start + i];
    }
    return false;
}
bool MemFs::MemFsNodeFile::write(const char *buf, size_t start, size_t num) {
    LockGuard l(_lock);
    while (_bytes.size() <= start + num) _bytes.emplace_back(0);
    for (size_t i = 0; i < num; i++) {
        _bytes[start + i] = buf[i];
    }
    return true;
}
size_t MemFs::MemFsNodeFile::size() {
    LockGuard l(_lock);
    return _bytes.size();
}
