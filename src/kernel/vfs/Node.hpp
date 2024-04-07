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

    Type          type() const { return _type; }
    const String &name() const {
        LockGuard l(_lock);
        return _name;
    }
    virtual SharedPtr<Node> traverse(const Path &path);

protected:
    Node(Type type) : _type(type) {}

    const Type _type = Type::INVALID;

    // This is uuugly
    mutable Mutex _lock;

    String        _name;
    Filesystem   *_mount = nullptr;
    WeakPtr<Node> _self_weak = nullptr;
};

class NodeFile;

class NodeDir : public Node {
public:
    virtual Vector<SharedPtr<Node>> children()                 = 0;
    virtual SharedPtr<NodeDir>         mkdir(const String &name)  = 0;
    virtual SharedPtr<NodeFile>         mkfile(const String &name) = 0;
    virtual void                    set_mounted(Filesystem *mount);

protected:
    NodeDir() : Node(Type::DIR) {}
};

class NodeFile : public Node {
public:
    virtual bool   read(char *buf, size_t start, size_t num)        = 0;
    virtual bool   write(const char *buf, size_t start, size_t num) = 0;
    virtual size_t size()                                           = 0;
    virtual bool   is_tty()                                         = 0;

protected:
    NodeFile() : Node(Type::FILE) {}
};


#endif //FICUS_NODE_HPP
