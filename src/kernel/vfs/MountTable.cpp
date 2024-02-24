//
// Created by Stepan Usatiuk on 23.02.2024.
//

#include "MountTable.hpp"
void MountTable::add_mount(Filesystem *fs) {
    _mounts.emplace_front(fs);
}
