//
// Created by Stepan Usatiuk on 24.02.2024.
//

#include "File.hpp"

#include "Node.hpp"

File::File(Node *node, FileOpts opts) : _n(node), _opts(opts) {
    if (opts & FileOpts::O_WRONLY)
        assert(opts & FileOpts::O_RDONLY);
    if (opts & FileOpts::O_WRONLY)
        while (!_n->lock_rw()) { yield_self(); }
    else
        while (!_n->lock_r()) { yield_self(); }
}
File::~File() {
    if (_opts & FileOpts::O_WRONLY)
        _n->unlock_rw();
    else
        _n->unlock_r();
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
    if (!(_opts & FileOpts::O_WRONLY)) return -1;
    if (file()) {
        file()->write(buf, _pos, size);
        _pos += size;
        return size;
    }
}
uint64_t File::size() {
    if (file()) return file()->size();
}
