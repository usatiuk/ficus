//
// Created by Stepan Usatiuk on 24.02.2024.
//

#include "VFSGlobals.hpp"
Vector<Node *> RootNode::children() {
    assert(false);
    return {};
}
NodeDir *RootNode::mkdir(const String &name) {
    assert(false);
    return nullptr;
}
NodeFile *RootNode::mkfile(const String &name) {
    assert(false);
    return nullptr;
}


RootNode VFSGlobals::root;
MountTable VFSGlobals::mounts;
