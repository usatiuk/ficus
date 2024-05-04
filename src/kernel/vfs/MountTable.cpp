//
// Created by Stepan Usatiuk on 23.02.2024.
//

#include "MountTable.hpp"
Filesystem *MountTable::add_mount(Filesystem *fs) {
    _mounts.emplace_front(fs);
    return fs;
}
