//
// Created by Stepan Usatiuk on 22.03.2024.
//

#ifndef OS2_DIRENT_H
#define OS2_DIRENT_H


#ifdef __cplusplus
#include <cstdint>
extern "C" {
#else
#include <stdint.h>
#endif

struct dirent {
    uint64_t inode_n;
    char d_name[];
};

#ifdef __cplusplus
}
#endif


#endif//OS2_DIRENT_H
