add_custom_target(iso_limine)

add_custom_target(iso
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/isodir/
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/limine.cfg ${CMAKE_CURRENT_BINARY_DIR}/isodir/
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:kernel> ${CMAKE_CURRENT_BINARY_DIR}/isodir/os2.elf
        COMMAND xorriso -as mkisofs -b limine-bios-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table --efi-boot limine-uefi-cd.bin -efi-boot-part --efi-boot-image --protective-msdos-label ${CMAKE_CURRENT_BINARY_DIR}/isodir -o ${CMAKE_CURRENT_BINARY_DIR}/os2.iso
        COMMAND  ${tools}/limine/prefix/bin/limine bios-install ${CMAKE_CURRENT_BINARY_DIR}/os2.iso
        VERBATIM
        DEPENDS kernel
        DEPENDS iso_limine
)


file(GLOB LIMINE_EFI_FILES ${tools}/limine/prefix/share/limine/*.EFI)
foreach (CurFile IN LISTS LIMINE_EFI_FILES)
    add_custom_command(
            TARGET iso_limine PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/isodir/EFI/BOOT/
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CurFile} ${CMAKE_CURRENT_BINARY_DIR}/isodir/EFI/BOOT/
            COMMENT "Copying limine efi file: ${CurFile}")
endforeach ()

file(GLOB LIMINE_BIN_FILES ${tools}/limine/prefix/share/limine/*.bin ${tools}/limine/prefix/share/limine/*.sys)
foreach (CurFile IN LISTS LIMINE_BIN_FILES)
    add_custom_command(
            TARGET iso_limine PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/isodir/
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CurFile} ${CMAKE_CURRENT_BINARY_DIR}/isodir/
            COMMENT "Copying limine bin file: ${CurFile}")
endforeach ()
