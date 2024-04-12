set(CMAKE_CXX_STANDARD 20)

add_executable(kernel.elf)

target_compile_options(kernel.elf PUBLIC $<$<COMPILE_LANGUAGE:CXX>:
        -fstack-protector-all
        -Wno-pointer-arith
        -fsanitize=undefined
        -ffreestanding
        -nostdlib
        -mno-red-zone
        -mcmodel=large
        -fno-exceptions
        -fno-rtti
        >)
target_compile_options(kernel.elf PUBLIC $<$<COMPILE_LANGUAGE:C>:
        -fstack-protector-all
        -Wno-pointer-arith
        -fsanitize=undefined
        -ffreestanding
        -nostdlib
        -mno-red-zone
        -mcmodel=large>)
target_link_options(kernel.elf PUBLIC
        -ffreestanding
        -nostdlib
        -mno-red-zone
        -mcmodel=large
        -fno-exceptions
        -fno-rtti)
target_include_directories(kernel.elf PRIVATE ${tools}/limine/prefix/include)


add_subdirectory(./arch/)
add_subdirectory(./kernel/)
add_subdirectory(./uapi/)


if (TEST_MODE)
    add_compile_options(-Wall -O0 -Wextra -pedantic -fno-omit-frame-pointer -Wshadow -Wformat=2 -Wfloat-equal -D_GLIBCXX_DEBUG -Wconversion -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2)
    add_compile_options(-fsanitize=address -fsanitize=undefined -fno-sanitize-recover)
    add_link_options(-fsanitize=address -fsanitize=undefined -fno-sanitize-recover)

    set_target_properties(kernel.elf PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)
    set_target_properties(syscalls_interface PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)
    add_subdirectory(./unit-tests/)
endif ()

if (NOT TEST_MODE)
    add_subdirectory(./test/)
    add_subdirectory(./iso/)
endif ()