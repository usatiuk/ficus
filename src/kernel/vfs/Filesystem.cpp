//
// Created by Stepan Usatiuk on 23.02.2024.
//

#include "Filesystem.hpp"

Filesystem::~Filesystem() = default;
Filesystem::Filesystem(NodeDir *mounted_on) : _mounted_on(mounted_on) {
    assert(_mounted_on->type() == Node::DIR);
    _mounted_on->set_mounted(this);
}
