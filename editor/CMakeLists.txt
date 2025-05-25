file(GLOB_RECURSE editor_src src/*.cpp)

add_executable(editor WIN32 ${editor_src})

target_link_libraries(editor PRIVATE windowing common_renderer opengl46 lua547 gamestate)