//
// Created by Stepan Usatiuk on 23.02.2024.
//

#ifndef FICUS_FILESYSTEM_HPP
#define FICUS_FILESYSTEM_HPP

#include "Node.hpp"

#include "PointersCollection.hpp"
#include "SkipList.hpp"

class Filesystem {
public:
    Filesystem()          = default;
    virtual ~Filesystem() = 0;

    Filesystem(Filesystem const &other)            = delete;
    Filesystem &operator=(Filesystem const &other) = delete;

    virtual SharedPtr<NodeDir> root() = 0;
    virtual SharedPtr<Node>    get_node(ino_t inode);

    void set_root(SharedPtr<NodeDir> node_dir);

protected:
    virtual SharedPtr<Node> get_node_impl(ino_t inode) = 0;

    SharedPtr<NodeDir> _mounted_on;

private:
    SkipListMap<ino_t, WeakPtr<Node>> _vnode_cache;
    Mutex                             _vnode_cache_lock;
};


#endif //FICUS_FILESYSTEM_HPP
