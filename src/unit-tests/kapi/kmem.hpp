//
// Created by Stepan Usatiuk on 10.04.2024.
//

#ifndef KMEM_HPP
#define KMEM_HPP

#include <cstddef>
#include <cstdint>
#include <cstdlib>

void *kmalloc(size_t n);
void  kfree(void *addr);
void *krealloc(void *addr, size_t newsize);


#endif //KMEM_HPP
