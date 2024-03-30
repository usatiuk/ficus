//
// Created by Stepan Usatiuk on 23.02.2024.
//

#ifndef FICUS_FDT_HPP
#define FICUS_FDT_HPP

#include "File.hpp"
#include "Node.hpp"
#include "Path.hpp"
#include "PointersCollection.hpp"
#include "SkipList.hpp"
#include "mutex.hpp"

class FDT {
public:
    using FD = int64_t;
    FD          open(const Path &p, FileOpts opts);
    void        close(FD fd);
    File       *get(FD fd) const;

    static FDT *current();

private:
    SkipList<FD, UniquePtr<File>> _files;
    int64_t                       _cur_fd = 10;
    mutable Mutex                 _mtx;
};

class FDHandle {
public:
    FDHandle(FDT::FD fd);
    ~FDHandle();
    FDHandle(const File &f)            = delete;
    FDHandle &operator=(const File &o) = delete;

    FDT::FD   get() { return _fd; }

private:
    FDT::FD _fd;
};


#endif //FICUS_FDT_HPP
