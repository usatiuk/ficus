//
// Created by Stepan Usatiuk on 24.02.2024.
//

#include "VFSGlobals.hpp"
Vector<SharedPtr<Node>> RootNode::children() {
    assert(false);
    return {};
}
SharedPtr<NodeDir> RootNode::mkdir(const String &name) {
    assert(false);
    return nullptr;
}
SharedPtr<NodeFile> RootNode::mkfile(const String &name) {
    assert(false);
    return nullptr;
}


RootNode   VFSGlobals::root;
MountTable VFSGlobals::mounts;
