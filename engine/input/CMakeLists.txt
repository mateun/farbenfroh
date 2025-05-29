file(GLOB_RECURSE input_src src/*.cpp)

add_library(input STATIC
        ${input_src}
)


target_include_directories(input PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(input PRIVATE windowing)
