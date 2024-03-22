//
// Created by Stepan Usatiuk on 23.02.2024.
//

#ifndef OS2_VFSAPI_HPP
#define OS2_VFSAPI_HPP


#include "FDT.hpp"
#include "Path.hpp"

namespace VFSApi {
    bool    mkdir(const Path &path);
    bool    touch(const Path &path);

    FDT::FD open(const Path &path);
    File   *get(FDT::FD fd);
    void    close(FDT::FD fd);

};     // namespace VFSApi


#endif //OS2_VFSAPI_HPP
