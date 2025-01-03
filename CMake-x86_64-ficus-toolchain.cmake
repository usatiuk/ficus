set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(triple x86_64-ficus)

set(tools ${FICUS_ROOT}/toolchain)
set(CMAKE_C_COMPILER ${tools}/gcc-x86_64-ficus-prefix/bin/x86_64-ficus-gcc)
set(CMAKE_CXX_COMPILER ${tools}/gcc-x86_64-ficus-prefix/bin/x86_64-ficus-g++)

set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
set(CMAKE_ASM_NASM_SOURCE_FILE_EXTENSIONS asm)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE NEVER)