add_library(assimp INTERFACE)

# Add include directory
target_include_directories(assimp INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../v2025/extlibs/assimp/Include
)

# Add link directory and library
target_link_directories(assimp INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../v2025/extlibs/assimp/Lib
)

target_link_libraries(assimp INTERFACE
        assimp-vc143-mt
)
