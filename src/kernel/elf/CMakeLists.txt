target_include_directories(kernel.elf PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(kernel.elf PRIVATE
        ElfParser.cpp
)

