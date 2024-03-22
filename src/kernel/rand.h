//
// Created by Stepan Usatiuk on 21.10.2023.
//

#ifndef OS2_RAND_H
#define OS2_RAND_H

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

#endif //OS2_RAND_H
