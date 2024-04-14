//
// Created by Stepan Usatiuk on 26.11.2023.
//

#include "limine_modules.hpp"

#include "assert.h"
#include "globals.hpp"
#include "paging.hpp"
#include "string.h"

volatile struct limine_module_request module_request = {
        .id       = LIMINE_MODULE_REQUEST,
        .revision = 0};

limine_file saved_modules[max_saved_modules];
char        saved_modules_names[max_saved_modules][max_saved_module_name] __attribute__((aligned(4096)));
unsigned    saved_modules_size = 0;

void limine_modules_remap() {
    for (int i = 0; i < module_request.response->module_count; i++) {
        assert2(i < max_saved_modules, "Too many modules");

        auto &mod = (*module_request.response->modules)[i];

        saved_modules_size++;
        saved_modules[i] = (*module_request.response->modules)[i];

        memcpy(saved_modules_names[i], mod.path, max_saved_module_name);

        for (size_t i = 0; i < PAGE_ROUND_UP(mod.size) / PAGE_SIZE; i++) {
            KERN_AddressSpace->map((void *) ((uintptr_t) mod.address + i * PAGE_SIZE), (void *) (BOOT_AddressSpace->virt2real(reinterpret_cast<void *>((uintptr_t) mod.address + i * PAGE_SIZE))), PAGE_RW);
        }
    }
}