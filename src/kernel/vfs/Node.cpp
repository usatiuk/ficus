//
// Created by Stepan Usatiuk on 23.02.2024.
//

#include "Node.hpp"
#include "LockGuard.hpp"

#include "Filesystem.hpp"

Node::~Node() = default;
Node *Node::traverse(const Path &path) {
    NodeDir    &nodeDir = static_cast<NodeDir &>(*this);

    Filesystem *mnt;
    {
        LockGuard l(_lock);
        mnt = nodeDir._mount;
    }
    if (mnt) return mnt->root()->traverse(path);

    if (path.empty()) {
        return this;
    }


    if (type() == DIR) {
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
    LockGuard l(_lock);
    assert(_type == DIR);
    assert(_mount == nullptr);
    _mount = mount;
}
