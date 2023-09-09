set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(triple x86_64-elf)

set(tools ${OS2_ROOT}/toolchain)
set(CMAKE_C_COMPILER ${tools}/gcc-x86_64-elf-prefix/bin/x86_64-elf-gcc)
set(CMAKE_CXX_COMPILER ${tools}/gcc-x86_64-elf-prefix/bin/x86_64-elf-g++)

set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
set(CMAKE_ASM_NASM_SOURCE_FILE_EXTENSIONS asm)

set(cxxflags -ffreestanding -nostdlib -mno-red-zone -mcmodel=large -fno-exceptions -fno-rtti)
set(cflags -ffreestanding -nostdlib -mno-red-zone -mcmodel=large)

add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:${cxxflags}>")
add_compile_options("$<$<COMPILE_LANGUAGE:C>:${cflags}>")

add_link_options(-ffreestanding -nostdlib -mno-red-zone -mcmodel=large -fno-exceptions -fno-rtti)

include_directories(${tools}/limine/prefix/include)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE NEVER)