//
// Created by Stepan Usatiuk on 26.11.2023.
//

#ifndef FICUS_LIMINE_MODULES_HPP
#define FICUS_LIMINE_MODULES_HPP

#include "limine.hpp"

extern volatile struct limine_module_request module_request;

static constexpr unsigned max_saved_modules          = 4;
static constexpr unsigned max_saved_module_name      = 256;

void limine_modules_remap();

extern unsigned           saved_modules_size;
extern limine_file        saved_modules[max_saved_modules];
extern char               saved_modules_names[max_saved_modules][max_saved_module_name] __attribute__((aligned(4096)));

#endif //FICUS_LIMINE_MODULES_HPP
