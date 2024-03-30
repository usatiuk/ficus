//
// Created by Stepan Usatiuk on 22.03.2024.
//

#ifndef FICUS_FILEOPTS_HPP
#define FICUS_FILEOPTS_HPP

#ifdef __cplusplus
#include <cstdint>
extern "C" {
#else
#include <stdint.h>
#endif

enum FileOpts : uint8_t {
    O_RDONLY = 1 << 1, // Read
    O_WRONLY = 1 << 2, // Write
    O_RDWR   = O_RDONLY | O_WRONLY,
    O_CREAT  = 1 << 3, // Create
};

#ifdef __cplusplus
}
#endif


#endif //FICUS_FILEOPTS_HPP
