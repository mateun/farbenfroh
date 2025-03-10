cmake_minimum_required(VERSION 3.26)
project(game_engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")

include_directories(src extlibs/glm-0.9.9.8/glm extlibs/ $ENV{VULKAN_SDK}/Include
                    extlibs/Recast/Include extlibs/Detour/Include extlibs/assimp/Include extlibs/lua547/src
        extlibs/glew-2.2.0/include extlibs)

link_directories(extlibs/jolt/Lib/Debug $ENV{VULKAN_SDK}/Lib extlibs/assimp/Lib extlibs/glew-2.2.0/lib)

add_definitions(-DJPH_PROFILE_ENABLED  -DJPH_DEBUG_RENDERER -DJPH_FLOATING_POINT_EXCEPTIONS_ENABLED
        -DJPH_USE_AVX2
        -DJPH_USE_AVX
        -DJPH_USE_SSE4_1
        -DJPH_USE_SSE4_2
        -DJPH_USE_LZCNT
        -DJPH_USE_TZCNT
        -DJPH_USE_F16C
        -DJPH_USE_FMADD)

#add_compile_definitions(-D _WITH_CONSOLE)

file(GLOB_RECURSE RECAST_SRC extlibs/Recast/Source/*.cpp)
file(GLOB_RECURSE DETOUR_SRC extlibs/Detour/Source/*.cpp)
file(GLOB_RECURSE LUA_SRC extlibs/lua547/src/*.c)

add_executable(game WIN32
        ${RECAST_SRC}
        ${DETOUR_SRC}
        ${LUA_SRC}
        src/imgui-build.cpp
        src/game-build.cpp

)

add_executable(animtest WIN32
        src/imgui-build.cpp
        src/engine/tests/test_units.cpp
        src/engine/tests/AnimationTests.cpp
)
add_executable(quadtree_test
        src/engine/algo/QuadTree.cpp
        src/engine/tests/QuadTreeTests.cpp

)

target_link_libraries(game dxgi.lib d3d11 d3dcompiler opengl32 gdi32 dsound dxguid assimp-vc143-mt Jolt vulkan-1 glew32)
target_link_libraries(animtest dsound dxgi opengl32 gdi32 dsound dxguid assimp-vc143-mt Jolt vulkan-1 glew32)