//
// Created by Stepan Usatiuk on 23.02.2024.
//

#include "Node.hpp"
#include "LockGuard.hpp"

#include "Filesystem.hpp"

Node::~Node() = default;
Node *Node::traverse(const Path &path) {
    LockGuardTry l(_lock);
    // FIXME: This is bad
    if (!l.locked() && _lock.owner() != cur_task())
        l.lock();

    NodeDir &nodeDir = static_cast<NodeDir &>(*this);
    if (nodeDir._mount) return nodeDir._mount->root()->traverse(path);
    if (path.empty()) return this;

    if (_type == DIR) {
        // Horribly inefficient
        auto children = nodeDir.children();
        for (size_t i = 0; i < children.size(); i++) {
            if (children[i]->name() == path[0]) {
                return children[i]->traverse(path.subvector(1, path.size()));
            }
        }
        return nullptr;
    }
    return nullptr;
}
void NodeDir::set_mounted(Filesystem *mount) {
    assert(_type == DIR);
    assert(_mount == nullptr);
    _mount = mount;
}
