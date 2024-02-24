target_include_directories(kernel.elf PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(kernel.elf PRIVATE
        Node.cpp
        MemFs.cpp
        Filesystem.cpp
        MountTable.cpp
        Path.cpp
        VFSTester.cpp
        VFSApi.cpp
        FDT.cpp
        VFSGlobals.cpp
        File.cpp
)