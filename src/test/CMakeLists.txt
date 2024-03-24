add_executable(init init.c)

target_link_libraries(init syscalls_interface)

target_include_directories(init PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_options(init PRIVATE "SHELL:-T${CMAKE_CURRENT_SOURCE_DIR}/linker.ld")
set_target_properties(init PROPERTIES LINK_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/linker.ld")

add_executable(hello2 hello2.c)

target_link_libraries(hello2 syscalls_interface)

target_include_directories(hello2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_options(hello2 PRIVATE "SHELL:-T${CMAKE_CURRENT_SOURCE_DIR}/linker.ld")
set_target_properties(hello2 PROPERTIES LINK_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/linker.ld")

