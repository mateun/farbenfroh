cmake_minimum_required(VERSION 3.26)
project(game_engine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_SYSTEM_VERSION 10.0.22621.0)


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
file(GLOB_RECURSE GAME_EDITOR_FLOATING_SRC src/game_editor_floating/*.cpp)
file(GLOB_RECURSE GAME_EDITOR_QT_SRC src/game_editor_qt/*.cpp)
file(GLOB_RECURSE ACTOR_MODEL_SRC src/engine/actor_model/*.cpp)


add_executable(command_repl WIN32
        ${GS_REPL_SRC}

)
target_link_libraries(command_repl dxgi.lib d3d11 d2d1 dwrite d3dcompiler gdiplus opengl32 gdi32 dsound dxguid assimp-vc143-mt Jolt vulkan-1 glew32 ozz_animation XInput)

#message(STATUS "Detected repl source files: ${GS_REPL_SRC}")

####################################################################
# GameEditor qt based
set(CMAKE_PREFIX_PATH "C:/Qt/6.9.0/msvc2022_64/lib/cmake"  )
#set(CMAKE_MODULE_PATH "C:/dev/lib/assimp/cmake-modules")
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};c:/dev/lib/assimp/installation-debug")
find_package(assimp CONFIG REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
add_executable(GameEditorQT ${GAME_EDITOR_QT_SRC} src/engine/dx11/dx11_impl.cpp ${ACTOR_MODEL_SRC})

target_link_libraries(GameEditorQT PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets d3dcompiler assimp::assimp)
target_compile_definitions(GameEditorQT PRIVATE QT_QPA_PLATFORM_PLUGIN_PATH=C:/Qt/6.9.0/msvc2022_64/plugins/platforms)
message(STATUS "Assimp Libs: ${ASSIMP_LIBRARIES}")
message(STATUS "MSVC_VERSION: ${MSVC_VERSION}")
#set(CMAKE_AUTOMOC ON)

# Optional: enable Qt's preprocessor (for signals/slots)
set_target_properties(GameEditorQT PROPERTIES
        AUTOMOC ON
        AUTORCC ON
        AUTOUIC ON
)

####################################################################

# BLANG Test:
file(GLOB_RECURSE BLANG_SRC src/engine/lang/*.cpp)
add_executable(lang_test ${BLANG_SRC})


####################################################################
# Game editor floating:
add_executable(game_editor_floating WIN32
        ${GAME_EDITOR_FLOATING_SRC}
        src/engine/io/io.cpp
)
#set_source_files_properties(resource.rc PROPERTIES COMPILE_FLAGS "")
set(CMAKE_MSVC_GENERATE_MANIFEST FALSE)
SET ( CMAKE_SHARED_LINKER_FLAGS /MANIFEST:NO )

target_link_libraries(game_editor_floating dxgi.lib d3d11 d2d1 comctl32 dwrite d3dcompiler gdiplus shlwapi opengl32 gdi32
        dsound dxguid assimp-vc143-mt Jolt vulkan-1 glew32 ozz_animation XInput uxtheme
            )


add_custom_command(TARGET
        game_editor_floating
        POST_BUILD
        COMMAND
        "mt.exe" -manifest \"${CMAKE_CURRENT_SOURCE_DIR}\\app.manifest\" -outputresource:\"${CMAKE_CURRENT_BINARY_DIR}\\game_editor_floating.exe\"\;\#2
        COMMENT
        "Adding custom manifest containing commctrl6.0 dependency..."
)
####################################################################





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


