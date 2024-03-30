//
// Created by Stepan Usatiuk on 23.02.2024.
//

#ifndef FICUS_MOUNTTABLE_HPP
#define FICUS_MOUNTTABLE_HPP

#include "String.hpp"

#include "Filesystem.hpp"
#include "Node.hpp"
#include "Path.hpp"

class MountTable {
public:
    void add_mount(Filesystem *fs);

private:
    List<Filesystem *> _mounts;
};


#endif //FICUS_MOUNTTABLE_HPP
