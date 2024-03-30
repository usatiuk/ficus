//
// Created by Stepan Usatiuk on 21.10.2023.
//

#ifndef FICUS_RAND_H
#define FICUS_RAND_H

#ifdef __cplusplus
extern "C" {
#endif

// The following functions define a portable implementation of rand and srand.
#define RAND_MAX 32767
int  rand(void);
void srand(unsigned int seed);

#ifdef __cplusplus
}
#endif

#endif //FICUS_RAND_H
