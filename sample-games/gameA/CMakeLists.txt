
file(GLOB_RECURSE gameA_src src/*.cpp)


add_executable(gameA WIN32 ${gameA_src})

target_link_libraries(gameA PRIVATE windowing common_renderer opengl46 lua547 gamestate)