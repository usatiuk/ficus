//
// Created by Stepan Usatiuk on 24.02.2024.
//

#include "File.hpp"

#include <utility>

#include "Node.hpp"

File::File(SharedPtr<Node> node, FileOpts opts) : _n(std::move(node)), _opts(opts) {
    if (opts & FileOpts::O_WRONLY)
        assert(opts & FileOpts::O_RDONLY);
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
        file()->read(buf, _pos, size);
        _pos += size;
        return size;
    }
}
uint64_t File::write(const char *buf, uint64_t size) {
    if (!(_opts & FileOpts::O_WRONLY)) return -1;
    if (file().get() != nullptr) {
        file()->write(buf, _pos, size);
        _pos += size;
        return size;
    }
}
uint64_t File::size() {
    if (file().get() != nullptr) return file()->size();
}
