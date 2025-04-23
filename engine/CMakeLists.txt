file(GLOB_RECURSE engine_src src/*.cpp)


add_library(engine SHARED
    ${engine_src}
)

target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(engine PRIVATE ENGINE_EXPORTS)
