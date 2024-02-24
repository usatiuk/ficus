//
// Created by Stepan Usatiuk on 23.02.2024.
//

#ifndef OS2_FILESYSTEM_HPP
#define OS2_FILESYSTEM_HPP

#include "Node.hpp"

class Filesystem {
public:
    Filesystem(NodeDir *mounted_on);

    virtual NodeDir *root() = 0;
    virtual ~Filesystem() = 0;

    NodeDir *_mounted_on;
};


#endif//OS2_FILESYSTEM_HPP
