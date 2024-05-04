//
// Created by Stepan Usatiuk on 23.02.2024.
//

#include "MemFs.hpp"
#include "LockGuard.hpp"

#include <algorithm>

Vector<DirEntry> MemFs::MemFsNodeDir::children() {
    LockGuard l(_lock);
    LockGuard l2(_fs_node->lock);

    Vector<DirEntry> out;
    for (auto c: _fs_node->entries) {
        out.emplace_back(c.first, c.second);
    }

    return out;
}

ino_t MemFs::MemFsNodeDir::mkdir(const String &name) {
    LockGuard l(_lock);

    MemFs *mem_fs = static_cast<MemFs *>(_fs);
    ino_t  res;
    {
        LockGuard l3(mem_fs->_files_lock);

        SharedPtr<DirInode> new_inode = SharedPtr<DirInode>(new DirInode{++mem_fs->_top_inode});
        mem_fs->_files.emplace(new_inode->id, static_ptr_cast<Inode>(new_inode));
        res = new_inode->id;
    }

    {
        LockGuard l2(_fs_node->lock);

        _fs_node->entries.emplace(name, res);
    }

    return res;
}
ino_t MemFs::MemFsNodeDir::mkfile(const String &name) {
    LockGuard l(_lock);

    MemFs *mem_fs = static_cast<MemFs *>(_fs);
    ino_t  res;
    {
        LockGuard l3(mem_fs->_files_lock);

        SharedPtr<FileInode> new_inode = SharedPtr<FileInode>(new FileInode{++mem_fs->_top_inode});
        mem_fs->_files.emplace(new_inode->id, static_ptr_cast<Inode>(new_inode));
        res = new_inode->id;
    }

    {
        LockGuard l2(_fs_node->lock);

        _fs_node->entries.emplace(name, res);
    }

    return res;
}
int64_t MemFs::MemFsNodeFile::read(char *buf, size_t start, size_t num) {
    LockGuard l(_lock);
    LockGuard l2(_fs_node->lock);
    num = std::min(num, _fs_node->data.size() - start);
    for (size_t i = 0; i < num; i++) {
        buf[i] = _fs_node->data[start + i];
    }
    return num;
}
int64_t MemFs::MemFsNodeFile::write(const char *buf, size_t start, size_t num) {
    LockGuard l(_lock);
    LockGuard l2(_fs_node->lock);
    while (_fs_node->data.size() <= start + num) _fs_node->data.emplace_back(0);
    for (size_t i = 0; i < num; i++) {
        _fs_node->data[start + i] = buf[i];
    }
    return num;
}
size_t MemFs::MemFsNodeFile::size() {
    LockGuard l(_lock);
    LockGuard l2(_fs_node->lock);
    return _fs_node->data.size();
}
MemFs::MemFs() {
    _files.emplace(1, new DirInode{1});
    _top_inode = 2;
}
SharedPtr<NodeDir> MemFs::root() {
    SharedPtr<Inode> root;
    {
        LockGuard l(_files_lock);
        root = _files.find(1)->second;
    }
    assert(root.get() != nullptr);
    {
        LockGuard l(root->lock);
        return static_ptr_cast<NodeDir>(MemFsNodeDir::create(this, static_ptr_cast<DirInode>(root)));
    }
}
SharedPtr<Node> MemFs::get_node_impl(ino_t inode) {
    LockGuard l(_files_lock);
    auto      found = _files.find(inode);
    if (found == _files.end()) return nullptr;
    if (found->second->type == InodeType::Dir) {
        return static_ptr_cast<Node>(MemFsNodeDir::create(this, static_ptr_cast<DirInode>(found->second)));
    } else if (found->second->type == InodeType::File) {
        return static_ptr_cast<Node>(MemFsNodeFile::create(this, static_ptr_cast<FileInode>(found->second)));
    }
    assert(false);
}


MemFs::Inode::~Inode() = default;