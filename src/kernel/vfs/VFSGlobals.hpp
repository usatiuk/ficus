//
// Created by Stepan Usatiuk on 24.02.2024.
//

#ifndef FICUS_VFSGLOBALS_HPP
#define FICUS_VFSGLOBALS_HPP

#include "MountTable.hpp"
#include "Node.hpp"

class RootNode : public NodeDir {
public:
    RootNode()
        : NodeDir(nullptr, -1) {}

    Vector<DirEntry> children() override;
    ino_t            mkdir(const String &name) override;
    ino_t            mkfile(const String &name) override;
};

namespace VFSGlobals {
    extern RootNode   root;
    extern MountTable mounts;
};     // namespace VFSGlobals


#endif //FICUS_VFSGLOBALS_HPP
