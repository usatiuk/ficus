//
// Created by Stepan Usatiuk on 23.02.2024.
//

#ifndef OS2_MOUNTTABLE_HPP
#define OS2_MOUNTTABLE_HPP

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


#endif //OS2_MOUNTTABLE_HPP
