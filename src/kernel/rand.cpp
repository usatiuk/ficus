//
// Created by Stepan Usatiuk on 21.10.2023.
//

#include <atomic>

#include "rand.h"

// The following functions define a portable implementation of rand and srand.

static std::atomic<unsigned long int> next = 1; // NB: "unsigned long int" is assumed to be 32 bits wide

extern "C" int rand(void)                       // RAND_MAX assumed to be 32767
{
    next = next * 1103515245 + 12345;
    return (unsigned int) (next / 65536ULL) % 32768;
}

extern "C" void srand(unsigned int seed) {
    next = seed;
}
