//
// Created by Stepan Usatiuk on 24.02.2024.
//

#ifndef OS2_FILE_HPP
#define OS2_FILE_HPP

#include "FileOpts.h"
#include <cstdint>

class Node;
class NodeDir;
class NodeFile;

class File {
public:
    File(Node *n, FileOpts opts);
    ~File();
    File(const File &f)                = delete;
    File     &operator=(const File &o) = delete;

    Node     *node();
    NodeDir  *dir();
    NodeFile *file() const;

    uint64_t  seek(uint64_t pos);
    uint64_t  read(char *buf, uint64_t size);
    uint64_t  write(const char *buf, uint64_t size);
    uint64_t  size();

private:
    Node *const _n;
    uint64_t    _pos = 0;
    FileOpts    _opts;
};

#endif //OS2_FILE_HPP
