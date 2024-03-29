
add_executable(kernel.elf)

target_compile_options(kernel.elf PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-fstack-protector-all -Wno-pointer-arith -fsanitize=undefined -ffreestanding -nostdlib -mno-red-zone -mcmodel=large -fno-exceptions -fno-rtti>)
target_compile_options(kernel.elf PUBLIC $<$<COMPILE_LANGUAGE:C>:-fstack-protector-all -Wno-pointer-arith -fsanitize=undefined -ffreestanding -nostdlib -mno-red-zone -mcmodel=large>)
target_link_options(kernel.elf PUBLIC -ffreestanding -nostdlib -mno-red-zone -mcmodel=large -fno-exceptions -fno-rtti)
target_include_directories(kernel.elf PRIVATE ${tools}/limine/prefix/include)


add_subdirectory(./arch/)
add_subdirectory(./kernel/)
add_subdirectory(./uapi/)

add_subdirectory(./test/)

add_subdirectory(./iso/)
