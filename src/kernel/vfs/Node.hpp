//
// Created by Stepan Usatiuk on 23.02.2024.
//

#ifndef FICUS_NODE_HPP
#define FICUS_NODE_HPP

#include <utility>

#include "List.hpp"
#include "LockGuard.hpp"
#include "String.hpp"

#include "Path.hpp"
#include "mutex.hpp"

class Filesystem;

class Node {
public:
    enum Type {
        FILE,
        DIR,
        INVALID
    };
    virtual ~Node() = 0;

    Type                    type() const { return _type; }
    virtual SharedPtr<Node> traverse(const Path &path);

protected:
    Node(Type type, Filesystem *fs, ino_t ino) : _type(type), _fs(fs), _ino(ino) {}

    const Type _type = Type::INVALID;

    mutable Mutex _lock;

    Filesystem *_mount = nullptr;
    Filesystem *_fs    = nullptr;

    ino_t _ino;

    // TODO: enable_shared_from_this or something prettier
    WeakPtr<Node> _self_weak = nullptr;
};

class NodeFile;

struct DirEntry {
    String name;
    ino_t  inode;
};

class NodeDir : public Node {
public:
    virtual Vector<DirEntry>    children()                 = 0;
    virtual ino_t               mkdir(const String &name)  = 0;
    virtual ino_t               mkfile(const String &name) = 0;
    virtual void                set_mounted(Filesystem *mount);

protected:
    NodeDir(Filesystem *fs, ino_t ino) : Node(Type::DIR, fs, ino) {}
};

class NodeFile : public Node {
public:
    virtual int64_t read(char *buf, size_t start, size_t num)        = 0;
    virtual int64_t write(const char *buf, size_t start, size_t num) = 0;
    virtual size_t  size()                                           = 0;
    virtual bool    is_tty()                                         = 0;

protected:
    NodeFile(Filesystem *fs, ino_t ino) : Node(Type::FILE, fs, ino) {}
};


#endif //FICUS_NODE_HPP
