
target_sources(kernel PRIVATE
        limine_mm.c
        mutex.c
        task.asm
        tty.c
        kmem.c
        kmain.asm
        paging.asm
        gdt.asm
        misc.asm
        limine_fb.c
        idt.c
        cv.c
        serial.c
        idt.asm
        globals.c
        memman.c
        timer.c
        boot.c
        io.c
        task.c
        paging.c
        kmain.c
        gdt.c
        misc.c)

target_include_directories(kernel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_options(kernel PRIVATE "SHELL:-T${CMAKE_CURRENT_SOURCE_DIR}/linker.ld")
set_target_properties(kernel PROPERTIES LINK_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/linker.ld")

