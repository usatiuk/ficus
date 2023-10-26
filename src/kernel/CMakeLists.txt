target_include_directories(kernel.elf PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(kernel.elf PRIVATE mutex.cpp cppsupport.cpp Spinlock.cpp LockGuard.cpp rand.cpp VMA.cpp asserts.cpp syscalls_interface.c)

add_subdirectory(templates)