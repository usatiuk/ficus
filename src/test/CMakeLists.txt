add_executable(hello hello.c)

target_link_libraries(hello syscalls_interface)

target_include_directories(hello PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_options(hello PRIVATE "SHELL:-T${CMAKE_CURRENT_SOURCE_DIR}/linker.ld")
set_target_properties(hello PROPERTIES LINK_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/linker.ld")

