//
// Created by Stepan Usatiuk on 23.02.2024.
//

#include "Filesystem.hpp"

Filesystem::~Filesystem() {
    if (_mounted_on.get()) {
        _mounted_on->set_mounted(nullptr);
    }
}

void Filesystem::set_root(SharedPtr<NodeDir> node) {
    _mounted_on = std::move(node);
    assert(_mounted_on->type() == Node::DIR);
    _mounted_on->set_mounted(this);
}

SharedPtr<Node> Filesystem::get_node(ino_t inode) {
    {
        LockGuard l(_vnode_cache_lock);
        if (auto p = _vnode_cache.find(inode); p != _vnode_cache.end())
            if (auto l = p->second.lock()) return *l;
    }

    auto found = get_node_impl(inode);

    {
        LockGuard l(_vnode_cache_lock);
        if (auto p = _vnode_cache.find(inode); p != _vnode_cache.end())
            if (auto l = p->second.lock()) return *l;

        _vnode_cache.emplace(inode, found);
        return found;
    }
}
