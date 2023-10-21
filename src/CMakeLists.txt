
add_executable(kernel.elf)

target_compile_options(kernel.elf PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions -fno-rtti -ffreestanding>)
target_compile_options(kernel.elf PUBLIC $<$<COMPILE_LANGUAGE:C>:-ffreestanding>)

add_subdirectory(./arch/)
add_subdirectory(./kernel/)

add_subdirectory(./iso/)
