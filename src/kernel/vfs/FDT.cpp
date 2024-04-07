//
// Created by Stepan Usatiuk on 23.02.2024.
//

#include "FDT.hpp"

#include "File.hpp"
#include "MountTable.hpp"
#include "PointersCollection.hpp"
#include "TtyPipe.hpp"
#include "VFSApi.hpp"
#include "VFSGlobals.hpp"
#include "paging.hpp"

FDT::FD FDT::open(const Path &p, FileOpts opts) {
    if (auto n = VFSGlobals::root.traverse(p); n.get() != nullptr) {
        LockGuard l(_mtx);
        _files.add(_cur_fd++, UniquePtr<File>(new File(n, opts)));
        return _cur_fd - 1;
    }
    if (opts & FileOpts::O_CREAT) {
        if (!VFSApi::touch(p)) return -1;
        return FDT::open(p, static_cast<FileOpts>(opts ^ FileOpts::O_CREAT));
    }
    return -1;
}

void FDT::close(FDT::FD fd) {
    LockGuard l(_mtx);
    if (auto f = _files.find(fd))
        if (!f->end)
            if (f->key == fd) {
                _files.erase(fd);
            }
}
File *FDT::get(FDT::FD fd) const {
    LockGuard l(_mtx);
    if (auto f = _files.find(fd))
        if (!f->end)
            if (f->key == fd)
                return f->data.get();
    return nullptr;
}

FDT *FDT::current() {
    return Scheduler::cur_task()->_addressSpace->getFdt();
}
FDT::FDT() {
    _files.add(0, UniquePtr(new File(static_ptr_cast<Node>(TtyPipe::create()), O_RDONLY)));
    _files.add(1, UniquePtr(new File(static_ptr_cast<Node>(TtyPipe::create()), O_RDWR)));
}
FDHandle::FDHandle(FDT::FD fd) : _fd(fd) {
}
FDHandle::~FDHandle() {
    FDT::current()->close(_fd);
}
