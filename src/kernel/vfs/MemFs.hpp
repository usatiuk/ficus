//
// Created by Stepan Usatiuk on 23.02.2024.
//

#ifndef FICUS_MEMFS_HPP
#define FICUS_MEMFS_HPP

#include "Filesystem.hpp"
#include "Node.hpp"

#include "SkipList.hpp"
#include "Vector.hpp"

class MemFs : public Filesystem {
    struct MemFsNodeDir : public NodeDir {
    public:
        Vector<SharedPtr<Node>>        children() override;
        SharedPtr<NodeDir>             mkdir(const String &name) override;
        SharedPtr<NodeFile>            mkfile(const String &name) override;

        static SharedPtr<MemFsNodeDir> create(const String &name) {
            auto shared        = SharedPtr(new MemFsNodeDir(name));
            shared->_self_weak = static_ptr_cast<Node>(shared);
            return shared;
        }

    private:
        MemFsNodeDir(const String &name) { _name = name; }
        SkipList<String, SharedPtr<Node>> _children;
    };

    struct MemFsNodeFile : public NodeFile {
    public:
        bool                            read(char *buf, size_t start, size_t num) override;
        bool                            write(const char *buf, size_t start, size_t num) override;
        size_t                          size() override;
        bool                            is_tty() override { return false; }

        static SharedPtr<MemFsNodeFile> create(const String &name) {
            auto shared        = SharedPtr(new MemFsNodeFile(name));
            shared->_self_weak = static_ptr_cast<Node>(shared);
            return shared;
        }

    private:
        MemFsNodeFile(const String &name) { _name = name; }
        Vector<uint8_t> _bytes;
    };

public:
    MemFs(NodeDir *mounted_on) : Filesystem(mounted_on) {}

    NodeDir *root() override { return _rootNode.get(); }

private:
    SharedPtr<MemFsNodeDir> _rootNode = MemFsNodeDir::create("");
};


#endif //FICUS_MEMFS_HPP
