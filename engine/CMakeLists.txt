
add_compile_definitions(ENGINE_EXPORTS)

include_directories(${CMAKE_SOURCE_DIR}/v2025/src)
add_subdirectory(windowing)
add_subdirectory(renderer)
