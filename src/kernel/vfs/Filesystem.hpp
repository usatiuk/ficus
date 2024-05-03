//
// Created by Stepan Usatiuk on 23.02.2024.
//

#ifndef FICUS_FILESYSTEM_HPP
#define FICUS_FILESYSTEM_HPP

#include "Node.hpp"

#include "PointersCollection.hpp"

class Filesystem {
public:
             Filesystem(NodeDir *mounted_on);
    virtual ~Filesystem() = 0;

    virtual SharedPtr<NodeDir> root()                = 0;
    virtual SharedPtr<Node>    get_node(ino_t inode) = 0;

    NodeDir *_mounted_on;
};


#endif //FICUS_FILESYSTEM_HPP
