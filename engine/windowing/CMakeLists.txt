file(GLOB_RECURSE windowing_src src/*.cpp)


add_library(windowing STATIC
    ${windowing_src}
)

target_compile_definitions(windowing PRIVATE ENGINE_EXPORTS)
