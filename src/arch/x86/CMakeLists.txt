
target_sources(kernel.elf PRIVATE
        limine_mm.cpp
        task.asm
        kmem.cpp
        kmain.asm
        paging.asm
        gdt.asm
        misc.asm
        limine_fb.cpp
        idt.cpp
        serial.cpp
        SerialTty.cpp
        idt.asm
        globals.cpp
        memman.cpp
        timer.cpp
        boot.cpp
        io.cpp
        task.cpp
        paging.cpp
        kmain.cpp
        gdt.cpp
        misc.cpp
        syscalls.cpp
        syscalls.asm
        limine_modules.cpp)

target_include_directories(kernel.elf PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_options(kernel.elf PRIVATE "SHELL:-T${CMAKE_CURRENT_SOURCE_DIR}/linker.ld")
set_target_properties(kernel.elf PROPERTIES LINK_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/linker.ld")

