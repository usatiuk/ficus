//
// Created by Stepan Usatiuk on 24.02.2024.
//

#ifndef OS2_VFSGLOBALS_HPP
#define OS2_VFSGLOBALS_HPP

#include "MountTable.hpp"
#include "Node.hpp"

class RootNode : public NodeDir {
public:
    Vector<Node *> children() override;
    NodeDir *mkdir(const String &name) override;
    NodeFile *mkfile(const String &name) override;
};

namespace VFSGlobals {
    extern RootNode root;
    extern MountTable mounts;
};// namespace VFSGlobals


#endif//OS2_VFSGLOBALS_HPP
