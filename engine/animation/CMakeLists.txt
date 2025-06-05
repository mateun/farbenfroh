file(GLOB_RECURSE animation_src src/*.cpp)

add_library(animation STATIC
        ${animation_src}
)


target_include_directories(animation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(animation PUBLIC engine_common)


