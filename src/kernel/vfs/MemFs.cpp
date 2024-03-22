//
// Created by Stepan Usatiuk on 23.02.2024.
//

#include "MemFs.hpp"
#include "LockGuard.hpp"

//FIXME: asserts on read also make sense
Vector<Node *> MemFs::MemFsNodeDir::children() {
    //    assert(!_rw_lock.test() || _rw_lock.owner() == cur_task());
    //    assert(_lock.owner() == cur_task());

    Vector<Node *> out;
    for (auto c: _children) {
        out.emplace_back(c.data);
    }
    return out;
}

NodeDir *MemFs::MemFsNodeDir::mkdir(const String &name) {
    assert(_rw_lock.owner() == cur_task());
    auto newnode = new MemFsNodeDir();
    newnode->_name = name;
    _children.add(name, newnode);
    return newnode;
}
NodeFile *MemFs::MemFsNodeDir::mkfile(const String &name) {
    assert(_rw_lock.owner() == cur_task());
    auto newfile = new MemFsNodeFile(name);
    _children.add(name, newfile);
    return newfile;
}
bool MemFs::MemFsNodeFile::read(char *buf, size_t start, size_t num) {
    assert(!_rw_lock.test() || _rw_lock.owner() == cur_task());
    //    assert(_lock.owner() == cur_task());
    if (start >= _bytes.size()) return false;
    if (start + num > _bytes.size()) return false;
    for (size_t i = 0; i < num; i++) {
        buf[i] = _bytes[start + i];
    }
    return false;
}
bool MemFs::MemFsNodeFile::write(const char *buf, size_t start, size_t num) {
    assert(_rw_lock.owner() == cur_task());
    // fixme
    while (_bytes.size() <= start + num) _bytes.emplace_back(0);
    for (size_t i = 0; i < num; i++) {
        _bytes[start + i] = buf[i];
    }
    return true;
}
size_t MemFs::MemFsNodeFile::size() {
    return _bytes.size();
}
