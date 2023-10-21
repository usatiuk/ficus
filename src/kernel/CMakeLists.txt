target_include_directories(kernel.elf PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(kernel.elf PRIVATE mutex.cpp cv.cpp cppsupport.cpp Spinlock.cpp LockGuard.cpp)

add_subdirectory(templates)