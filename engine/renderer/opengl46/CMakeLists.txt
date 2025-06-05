file(GLOB_RECURSE opengl46_src src/*.cpp)

add_library(opengl46 STATIC
        ${opengl46_src}
)

target_link_directories(opengl46 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../../v2025/extlibs/glew-2.2.0/lib  )
target_include_directories(opengl46 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../../v2025/extlibs/glew-2.2.0/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../v2025/src/engine/io )
target_link_libraries(opengl46 common_renderer engine_common glew32 opengl32  animation )

#target_link_libraries(vulkan engine_common vulkan-1 util common_renderer shaderc_combinedd)

add_custom_command(TARGET opengl46 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../v2025/extlibs/glew-2.2.0/bin/glew32.dll

        ${CMAKE_BINARY_DIR}/bin
)