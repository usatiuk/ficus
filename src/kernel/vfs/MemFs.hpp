//
// Created by Stepan Usatiuk on 23.02.2024.
//

#ifndef OS2_MEMFS_HPP
#define OS2_MEMFS_HPP

#include "Filesystem.hpp"
#include "Node.hpp"

#include "SkipList.hpp"
#include "Vector.hpp"

class MemFs : public Filesystem {
    struct MemFsNodeDir : public NodeDir {
    public:
        Vector<Node *> children() override;
        NodeDir *mkdir(const String &name) override;
        NodeFile *mkfile(const String &name) override;

    private:
        SkipList<String, Node *> _children;
    };

    struct MemFsNodeFile : public NodeFile {
    public:
        MemFsNodeFile(const String &name) { _name = name; }

        bool read(char *buf, size_t start, size_t num) override;
        bool write(const char *buf, size_t start, size_t num) override;
        size_t size() override;

    private:
        Vector<uint8_t> _bytes;
    };

public:
    MemFs(NodeDir *mounted_on) : Filesystem(mounted_on) {}

    NodeDir *root() override { return &_rootNode; }

private:
    MemFsNodeDir _rootNode;
};


#endif//OS2_MEMFS_HPP
