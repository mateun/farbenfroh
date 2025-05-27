cmake_minimum_required(VERSION 3.26)
project(game_engine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_SYSTEM_VERSION 10.0.22621.0)

# Common output directory
set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin)

# Apply to all targets globally
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})  # .exe, .dll (runtime)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})  # .dll (if treated as library)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})  # .lib, .a (static libs)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/engine/include
       ${CMAKE_CURRENT_SOURCE_DIR}/v2025/extlibs/glm-0.9.9.8/glm)


add_subdirectory(engine)
add_subdirectory(sample-games)