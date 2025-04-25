file(GLOB_RECURSE common_src src/*.cpp)

add_library(common_renderer SHARED
        ${common_src}
)

