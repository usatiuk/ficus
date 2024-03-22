//
// Created by Stepan Usatiuk on 23.02.2024.
//

#include "Node.hpp"
#include "LockGuard.hpp"

#include "Filesystem.hpp"

Node::~Node() = default;
Node *Node::traverse(const Path &path) {
    while (!lock_r()) { yield_self(); }

    NodeDir &nodeDir = static_cast<NodeDir &>(*this);
    if (nodeDir._mount) return nodeDir._mount->root()->traverse(path);
    if (path.empty()) {
        unlock_r();
        return this;
    }

    if (_type == DIR) {
        // Horribly inefficient
        auto children = nodeDir.children();
        for (size_t i = 0; i < children.size(); i++) {
            if (children[i]->name() == path[0]) {
                unlock_r();
                return children[i]->traverse(path.subvector(1, path.size()));
            }
        }
        unlock_r();
        return nullptr;
    }
    unlock_r();
    return nullptr;
}

void NodeDir::set_mounted(Filesystem *mount) {
    assert(_type == DIR);
    assert(_mount == nullptr);
    _mount = mount;
}

bool Node::lock_r() {
    LockGuard l(_lock);

    if (_rw_lock.test() && _rw_lock.owner() != cur_task()) return false;
    _r_lock_count++;

    return true;
}
bool Node::lock_rw() {
    LockGuard l(_lock);

    if (_rw_lock.test() && _rw_lock.owner() != cur_task()) return false;

    if (_r_lock_count != 0 && !(_rw_lock.test() && _rw_lock.owner() == cur_task())) return false;

    if (_rw_lock_count == 0) {
        assert(!_rw_lock.test());
        _rw_lock.lock();
    }

    _rw_lock_count++;

    return true;
}
void Node::unlock_r() {
    LockGuard l(_lock);
    assert(!_rw_lock.test() || _rw_lock.owner() == cur_task());
    assert(_r_lock_count > 0);
    _r_lock_count--;
}
void Node::unlock_rw() {
    LockGuard l(_lock);
    assert(_rw_lock.test() && _rw_lock.owner() == cur_task());
    assert(_rw_lock_count > 0);
    _rw_lock_count--;
    if (_rw_lock_count == 0)
        _rw_lock.unlock();
}
