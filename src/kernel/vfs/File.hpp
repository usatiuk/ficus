//
// Created by Stepan Usatiuk on 24.02.2024.
//

#ifndef FICUS_FILE_HPP
#define FICUS_FILE_HPP

#include "PointersCollection.hpp"

#include <cstdint>

class Node;
class NodeDir;
class NodeFile;

class File {
public:
    File(SharedPtr<Node> n, int opts);
    ~File();
    File(const File &f)            = delete;
    File &operator=(const File &o) = delete;

    SharedPtr<Node>     node() const;
    SharedPtr<NodeDir>  dir() const;
    SharedPtr<NodeFile> file() const;

    uint64_t seek(uint64_t pos);
    uint64_t pos() { return _pos; }
    uint64_t read(char *buf, uint64_t size);
    uint64_t write(const char *buf, uint64_t size);
    uint64_t size();

private:
    SharedPtr<Node> _n;
    uint64_t        _pos = 0;
    int             _opts;
};

#endif //FICUS_FILE_HPP
