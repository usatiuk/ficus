add_executable(init init.c)

target_compile_options(init PRIVATE -fno-exceptions -fno-rtti)
target_link_libraries(init syscalls_interface)

target_include_directories(init PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(hello2 hello2.c)

