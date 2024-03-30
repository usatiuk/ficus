//
// Created by Stepan Usatiuk on 22.03.2024.
//

#ifndef FICUS_DIRENT_H
#define FICUS_DIRENT_H


#ifdef __cplusplus
#include <cstdint>
extern "C" {
#else
#include <stdint.h>
#endif

struct dirent {
    uint64_t inode_n;
    char     d_name[];
};

#ifdef __cplusplus
}
#endif


#endif //FICUS_DIRENT_H
