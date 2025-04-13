cmake_minimum_required(VERSION 3.26)
project(game_engine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")

add_subdirectory(extlibs/ozz extlibs/ozz/out/)

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
file(GLOB_RECURSE ENGINE_SRC src/engine/*.cpp)
file(GLOB_RECURSE GAME_RAW_SRC games/opengl/test_terrain/*.cpp)
file(GLOB_RECURSE GAME_FX_SRC games/opengl/GameEditor/*.cpp)
file(GLOB_RECURSE GS_REPL_SRC src/game_shell/repl/*.cpp)


add_executable(command_repl WIN32
        ${GS_REPL_SRC}
)

target_link_libraries(command_repl dxgi.lib d3d11 d2d1 dwrite d3dcompiler gdiplus opengl32 gdi32 dsound dxguid assimp-vc143-mt Jolt vulkan-1 glew32 ozz_animation XInput)
message(STATUS "Detected repl source files: ${GS_REPL_SRC}")

add_executable(game WIN32
        ${RECAST_SRC}
        ${DETOUR_SRC}
        ${LUA_SRC}
        ${ENGINE_SRC}
        src/imgui-build.cpp

)
target_compile_definitions(game PRIVATE USE_RAW_WIN32=1)

add_executable(game_with_app_framework WIN32
        ${ENGINE_SRC}
        ${GAME_FX_SRC}
        src/imgui-build.cpp
)

target_compile_definitions(game_with_app_framework PRIVATE USE_WIN32_APP_FRAMEWORK=1)

add_executable(animtest WIN32
        ${ENGINE_SRC}
        src/imgui-build.cpp
        src/engine/tests/AnimationTests.cpp

)
target_compile_definitions(animtest PRIVATE USE_RAW_WIN32=1)

add_executable(quadtree_test
        ${ENGINE_SRC}
        src/engine/algo/QuadTree.cpp
        src/engine/tests/QuadTreeTests.cpp
)
target_compile_definitions(quadtree_test PRIVATE USE_RAW_WIN32=1)

add_executable(collisiongrid_test
        ${ENGINE_SRC}
        src/engine/algo/CollisionGrid.cpp
        src/engine/tests/CollisionGridTests.cpp
)
target_compile_definitions(collisiongrid_test PRIVATE COLLISION_GRID_TESTS=1)





target_link_libraries(game dxgi.lib d3d11 d3dcompiler opengl32 gdi32 dsound dxguid assimp-vc143-mt Jolt vulkan-1 glew32 ozz_animation XInput)
target_link_libraries(game_with_app_framework dxgi dxguid opengl32 gdiplus comctl32 gdi32 dsound assimp-vc143-mt Jolt vulkan-1 glew32 ozz_animation XInput)
target_link_libraries(animtest dsound dxgi opengl32 gdi32 dsound dxguid assimp-vc143-mt Jolt vulkan-1 glew32 ozz_animation)


