//
// Created by Stepan Usatiuk on 24.02.2024.
//

#include "File.hpp"

#include <utility>

#include "Node.hpp"

#include <sys/fcntl.h>

File::File(SharedPtr<Node> node, int opts) : _n(std::move(node)), _opts(opts) {
}
File::~File() {
}
SharedPtr<Node> File::node() const {
    return _n;
}
SharedPtr<NodeDir> File::dir() const {
    if (_n.get() && _n->type() == Node::DIR) return static_ptr_cast<NodeDir>(_n);
    return nullptr;
}
SharedPtr<NodeFile> File::file() const {
    if (_n.get() && _n->type() == Node::FILE) return static_ptr_cast<NodeFile>(_n);
    return nullptr;
}
uint64_t File::seek(uint64_t pos) {
    _pos = pos;
    return pos;
}
uint64_t File::read(char *buf, uint64_t size) {
    if (file().get() != nullptr) {
        int64_t fret = file()->read(buf, _pos, size);
        _pos += fret;
        return fret;
    }
}
uint64_t File::write(const char *buf, uint64_t size) {
    if (!(_opts & O_WRONLY) && !(_opts & O_RDWR)) return -1;
    if (file().get() != nullptr) {
        int64_t fret = file()->write(buf, _pos, size);
        _pos += fret;
        return fret;
    }
}
uint64_t File::size() {
    if (file().get() != nullptr) return file()->size();
}
