file(GLOB_RECURSE opengl46_src src/*.cpp)



add_library(opengl46 SHARED
        ${opengl46_src}
)

target_link_directories(opengl46 PUBLIC ${CMAKE_SOURCE_DIR}/v2025/extlibs/glew-2.2.0/lib  )
target_include_directories(opengl46 PRIVATE ${CMAKE_SOURCE_DIR}/v2025/extlibs/glew-2.2.0/include ${CMAKE_SOURCE_DIR}/v2025/src/engine/io )
target_link_libraries(opengl46 PUBLIC glew32 opengl32 common_renderer )

add_custom_command(TARGET opengl46 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/v2025/extlibs/glew-2.2.0/bin/glew32.dll

        ${CMAKE_BINARY_DIR}/bin
)