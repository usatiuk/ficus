//
// Created by Stepan Usatiuk on 24.02.2024.
//

#include "VFSGlobals.hpp"
Vector<DirEntry> RootNode::children() {
    assert(false);
    return {};
}
ino_t RootNode::mkdir(const String &name) {
    assert(false);
    return -1;
}
ino_t RootNode::mkfile(const String &name) {
    assert(false);
    return -1;
}


SharedPtr<RootNode> VFSGlobals::root;
MountTable          VFSGlobals::mounts;
