//
// Created by Stepan Usatiuk on 23.02.2024.
//

#include "FDT.hpp"

#include "File.hpp"
#include "MountTable.hpp"
#include "PointersCollection.hpp"
#include "VFSGlobals.hpp"
#include "paging.hpp"

FDT::FD FDT::open(const Path &p) {
    if (auto n = VFSGlobals::root.traverse(p)) {
        _files.add(_cur_fd++, UniquePtr<File>(new File(n)));
        return _cur_fd - 1;
    }
    return -1;
}

void FDT::close(FDT::FD fd) {
    if (auto f = _files.find(fd))
        if (!f->end)
            if (f->key == fd) {
                _files.erase(fd);
            }
}
File *FDT::get(FDT::FD fd) {
    if (auto f = _files.find(fd))
        if (!f->end)
            if (f->key == fd)
                return f->data.get();
    return nullptr;
}

FDT *FDT::current() {
    return cur_task()->addressSpace->getFdt();
}
FDHandle::FDHandle(FDT::FD fd) : _fd(fd) {
}
FDHandle::~FDHandle() {
    FDT::current()->close(_fd);
}
