//
// Created by Stepan Usatiuk on 26.11.2023.
//

#ifndef OS2_LIMINE_MODULES_HPP
#define OS2_LIMINE_MODULES_HPP

#include "limine.hpp"

void                      limine_modules_save();

static constexpr unsigned max_saved_modules          = 2;
static constexpr unsigned max_saved_module_file_size = 1024 * 1024;
static constexpr unsigned max_saved_module_name      = 256;

extern unsigned           saved_modules_size;
extern limine_file        saved_modules[max_saved_modules];
extern char               saved_modules_data[max_saved_modules][max_saved_module_file_size] __attribute__((aligned(4096)));
extern uint64_t           saved_modules_data_size[max_saved_modules] __attribute__((aligned(4096)));
extern char               saved_modules_names[max_saved_modules][max_saved_module_name] __attribute__((aligned(4096)));

#endif //OS2_LIMINE_MODULES_HPP
