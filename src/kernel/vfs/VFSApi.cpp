//
// Created by Stepan Usatiuk on 23.02.2024.
//

#include "VFSApi.hpp"

#include "File.hpp"
#include "Node.hpp"

bool VFSApi::mkdir(const Path &path) {
    auto root = path.subvector(0, path.size() - 1);
    FDHandle root_fd = FDHandle(FDT::current()->open(root, O_RDWR));
    if (root_fd.get() == -1) return false;
    File *root_f = FDT::current()->get(root_fd.get());
    if (!root_f->dir()) return false;
    root_f->dir()->mkdir(path.back());
    return true;
}
bool VFSApi::touch(const Path &path) {
    auto root = path.subvector(0, path.size() - 1);
    FDHandle root_fd = FDHandle(FDT::current()->open(root, O_RDWR));
    if (root_fd.get() == -1) return false;
    File *root_f = FDT::current()->get(root_fd.get());
    if (!root_f->dir()) return false;
    root_f->dir()->mkfile(path.back());
    return true;
}

FDT::FD VFSApi::open(const Path &path) {
    return FDT::current()->open(path, O_RDWR);
}
void VFSApi::close(FDT::FD fd) {
    return FDT::current()->close(fd);
}
File *VFSApi::get(FDT::FD fd) {
    return FDT::current()->get(fd);
}
