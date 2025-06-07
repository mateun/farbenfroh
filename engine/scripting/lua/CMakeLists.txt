file(GLOB_RECURSE lua547_src ${CMAKE_CURRENT_SOURCE_DIR}/../../../v2025/extlibs/lua547/src/*.c)
file(GLOB_RECURSE lua_util_src src/*.cpp)

list(REMOVE_ITEM lua547_src
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../v2025/extlibs/lua547/src/lua.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../v2025/extlibs/lua547/src/luac.c
)



add_library(lua547 STATIC
        ${lua547_src}
        ${lua_util_src}
)

target_include_directories(lua547
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../../v2025/extlibs/lua547/src
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(lua547 PRIVATE gamestate)
