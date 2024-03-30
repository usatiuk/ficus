//
// Created by Stepan Usatiuk on 24.02.2024.
//

#ifndef FICUS_VFSGLOBALS_HPP
#define FICUS_VFSGLOBALS_HPP

#include "MountTable.hpp"
#include "Node.hpp"

class RootNode : public NodeDir {
public:
    Vector<Node *> children() override;
    NodeDir       *mkdir(const String &name) override;
    NodeFile      *mkfile(const String &name) override;
};

namespace VFSGlobals {
    extern RootNode   root;
    extern MountTable mounts;
};     // namespace VFSGlobals


#endif //FICUS_VFSGLOBALS_HPP
