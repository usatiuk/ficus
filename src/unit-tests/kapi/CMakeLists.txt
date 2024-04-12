add_library(KApi
        kmem.cpp
)

target_include_directories(KApi INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})