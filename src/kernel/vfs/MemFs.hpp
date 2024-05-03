//
// Created by Stepan Usatiuk on 23.02.2024.
//

#ifndef FICUS_MEMFS_HPP
#define FICUS_MEMFS_HPP

#include <utility>

#include "Filesystem.hpp"
#include "Node.hpp"

#include "SkipList.hpp"
#include "Vector.hpp"

class MemFs : public Filesystem {
private:
    enum class InodeType {
        File,
        Dir
    };

    // FIXME: refcount
    struct Inode {
        explicit Inode(InodeType type, ino_t id)
            : type(type), id(id) {}
        virtual ~Inode() = 0;

        ino_t         id;
        InodeType     type;
        mutable Mutex lock;
    };

    struct FileInode final : public Inode {
        explicit FileInode(ino_t id)
            : Inode(InodeType::File, id) {}

        ~FileInode() override = default;

        Vector<uint8_t> data;
    };

    struct DirInode final : public Inode {
        explicit DirInode(ino_t id)
            : Inode(InodeType::Dir, id) {}

        ~DirInode() override = default;

        SkipListMap<String, ino_t> entries;
    };

    struct MemFsNodeDir : public NodeDir {
    public:
        ~MemFsNodeDir() override = default;

        Vector<DirEntry> children() override;
        ino_t            mkdir(const String &name) override;
        ino_t            mkfile(const String &name) override;

        static SharedPtr<MemFsNodeDir> create(Filesystem *fs, SharedPtr<DirInode> fs_node) {
            auto shared        = SharedPtr(new MemFsNodeDir(fs, std::move(fs_node)));
            shared->_self_weak = static_ptr_cast<Node>(shared);
            return shared;
        }

    private:
        MemFsNodeDir(Filesystem *fs, SharedPtr<DirInode> fs_node) : NodeDir(fs, fs_node->id), _fs_node(std::move(fs_node)) {}

        SharedPtr<DirInode> _fs_node;
    };

    struct MemFsNodeFile : public NodeFile {
    public:
        ~MemFsNodeFile() override = default;

        int64_t read(char *buf, size_t start, size_t num) override;
        int64_t write(const char *buf, size_t start, size_t num) override;
        size_t  size() override;
        bool    is_tty() override { return false; }

        static SharedPtr<MemFsNodeFile> create(Filesystem *fs, SharedPtr<FileInode> fs_node) {
            auto shared        = SharedPtr(new MemFsNodeFile(fs, std::move(fs_node)));
            shared->_self_weak = static_ptr_cast<Node>(shared);
            return shared;
        }

    private:
        MemFsNodeFile(Filesystem *fs, SharedPtr<FileInode> fs_node) : NodeFile(fs, fs_node->id), _fs_node(std::move(fs_node)) {}

        SharedPtr<FileInode> _fs_node;
    };

public:
    MemFs(NodeDir *mounted_on);

    SharedPtr<NodeDir> root() override;
    SharedPtr<Node>    get_node(ino_t inode) override;

private:
    ino_t                                _top_inode;
    SkipListMap<ino_t, SharedPtr<Inode>> _files;
    Mutex                                _files_lock;
};


#endif //FICUS_MEMFS_HPP
