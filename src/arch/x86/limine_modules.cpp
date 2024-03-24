//
// Created by Stepan Usatiuk on 26.11.2023.
//

#include "limine_modules.hpp"

#include "asserts.hpp"
#include "string.h"

static volatile struct limine_module_request module_request = {
        .id       = LIMINE_MODULE_REQUEST,
        .revision = 0};


limine_file saved_modules[max_saved_modules];
char        saved_modules_data[max_saved_modules][max_saved_module_file_size] __attribute__((aligned(4096)));
uint64_t    saved_modules_data_size[max_saved_modules] __attribute__((aligned(4096)));
char        saved_modules_names[max_saved_modules][max_saved_module_name] __attribute__((aligned(4096)));
unsigned    saved_modules_size = 0;

void        limine_modules_save() {
    for (int i = 0; i < module_request.response->module_count; i++) {
        assert(i < max_saved_modules);
        saved_modules_size++;
        saved_modules[i] = (*module_request.response->modules)[i];
        assert(saved_modules[i].size < max_saved_module_file_size);
        memcpy(saved_modules_data[i], saved_modules[i].address, saved_modules[i].size);
        memcpy(saved_modules_names[i], saved_modules[i].path, max_saved_module_name);
        saved_modules_data_size[i] = saved_modules[i].size;
    }
}