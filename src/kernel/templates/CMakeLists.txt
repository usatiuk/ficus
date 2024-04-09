
add_library(templates
        INTERFACE
)
target_include_directories(templates INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(kernel.elf PRIVATE templates)

add_subdirectory(stl)