file(GLOB_RECURSE common_src src/*.cpp)

add_library(common_renderer STATIC
        ${common_src}
)

target_include_directories(common_renderer
        PUBLIC
        include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../v2025/extlibs/glm-0.9.9.8/glm
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../v2025/src/engine/io
)

target_link_libraries(common_renderer PRIVATE engine_common animation util)
