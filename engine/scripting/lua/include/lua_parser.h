//
// Created by mgrus on 25.05.2025.
//

#ifndef LUA_PARSER_H
#define LUA_PARSER_H
#include <vector>


struct Entity;
struct lua_State;

void execute_lua_file(lua_State* L, const std::string& filename);
std::vector<Entity> load_entities(lua_State* state, const char* filename);
std::vector<UIElement> load_ui(lua_State* L, const char* filename);


#endif //LUA_PARSER_H
