//
// Created by Stepan Usatiuk on 24.02.2024.
//

#include "File.hpp"

#include "Node.hpp"

File::File(Node *node) : _n(node) {
    _n->lock();
}
File::~File() {
    _n->unlock();
}
Node *File::node() {
    return _n;
}
NodeDir *File::dir() {
    if (_n && _n->type() == Node::DIR) return static_cast<NodeDir *>(_n);
    return nullptr;
}
NodeFile *File::file() {
    if (_n && _n->type() == Node::FILE) return static_cast<NodeFile *>(_n);
    return nullptr;
}
uint64_t File::seek(uint64_t pos) {
    _pos = pos;
    return pos;
}
uint64_t File::read(char *buf, uint64_t size) {
    if (file()) {
        file()->read(buf, _pos, size);
        _pos += size;
        return size;
    }
}
uint64_t File::write(const char *buf, uint64_t size) {
    if (file()) {
        file()->write(buf, _pos, size);
        _pos += size;
        return size;
    }
}
uint64_t File::size() {
    if (file()) return file()->size();
}
