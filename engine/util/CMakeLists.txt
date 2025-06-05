file(GLOB_RECURSE util_src src/*.cpp)


add_library(util STATIC
        ${util_src}
)

target_include_directories(util
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(util PRIVATE engine_common gamestate)
