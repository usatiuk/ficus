
add_executable(kernel.elf)

target_compile_options(kernel.elf PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-fstack-protector-all -Wno-pointer-arith -fsanitize=undefined>)
target_compile_options(kernel.elf PUBLIC $<$<COMPILE_LANGUAGE:C>:-fstack-protector-all -Wno-pointer-arith -fsanitize=undefined>)

add_subdirectory(./arch/)
add_subdirectory(./kernel/)
add_subdirectory(./uapi/)

add_subdirectory(./test/)

add_subdirectory(./iso/)
