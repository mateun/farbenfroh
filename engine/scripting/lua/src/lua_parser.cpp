//
// Created by mgrus on 25.05.2025.
//

#include <lua.hpp>
#include <iostream>
#include <vector>
#include <gamestate.h>

void execute_lua_file(lua_State* L, const std::string& filename) {
    auto result = luaL_dofile(L, filename.c_str());
    if (result != LUA_OK) {
        const char* err = lua_tostring(L, -1);
        std::cerr << "Lua error: " << err << std::endl;
        lua_pop(L, 1); // remove error message from the stack
        exit(13433);
    }
}

std::vector<UIElement> load_ui(lua_State* L, const char* filename) {
    std::vector<UIElement> elements;

    if (luaL_dofile(L, filename) != LUA_OK) {
        std::cerr << "Lua error: " << lua_tostring(L, -1) << "\n";
        lua_close(L);
        return elements;
    }

    if (!lua_istable(L, -1)) {
        std::cerr << "Expected table from Lua file\n";
        lua_close(L);
        return elements;
    }

    lua_getfield(L, -1, "ui");
    if (!lua_istable(L, -1)) {
        std::cerr << "'ui' section missing or not a table\n";
        lua_close(L);
        return elements;
    }

    lua_pushnil(L); // for lua_next
    while (lua_next(L, -2)) {
        if (lua_istable(L, -1)) {
            UIElement e;

            lua_getfield(L, -1, "id");
            if (lua_isstring(L, -1)) e.id = lua_tostring(L, -1);
            lua_pop(L, 1);

            lua_getfield(L, -1, "type");
            if (lua_isstring(L, -1)) e.type = lua_tostring(L, -1);
            lua_pop(L, 1);

            lua_getfield(L, -1, "text");
            if (lua_isstring(L, -1)) e.text = lua_tostring(L, -1);
            lua_pop(L, 1);

            lua_getfield(L, -1, "position");
            if (lua_istable(L, -1)) {
                for (int i = 0; i < 2; ++i) {
                    lua_rawgeti(L, -1, i + 1);
                    if (lua_isnumber(L, -1)) {
                        e.position[i] = static_cast<float>(lua_tonumber(L, -1));
                    }
                    lua_pop(L, 1);
                }
            }
            lua_pop(L, 1);

            lua_getfield(L, -1, "size");
            if (lua_istable(L, -1)) {
                for (int i = 0; i < 2; ++i) {
                    lua_rawgeti(L, -1, i + 1);
                    if (lua_isnumber(L, -1)) {
                        e.size[i] = static_cast<float>(lua_tonumber(L, -1));
                    }
                    lua_pop(L, 1);
                }
            }
            lua_pop(L, 1);

            lua_getfield(L, -1, "anchor");
            if (lua_isstring(L, -1)) e.anchor = lua_tostring(L, -1);
            lua_pop(L, 1);

            lua_getfield(L, -1, "font_size");
            if (lua_isinteger(L, -1)) e.font_size = static_cast<int>(lua_tointeger(L, -1));
            lua_pop(L, 1);

            elements.push_back(e);
        }

        lua_pop(L, 1); // pop value, keep key
    }

    return elements;
}

std::vector<Entity> load_entities(lua_State* L, const char* filename) {
    std::vector<Entity> entities;

    if (luaL_dofile(L, filename) != LUA_OK) {
        std::cerr << "Lua error: " << lua_tostring(L, -1) << "\n";
        lua_close(L);
        return entities;
    }

    // Returned table is now on top of stack
    if (!lua_istable(L, -1)) {
        std::cerr << "Expected table from Lua file\n";
        lua_close(L);
        return entities;
    }

    // Get "entities" field
    lua_getfield(L, -1, "entities");
    if (!lua_istable(L, -1)) {
        std::cerr << "'entities' is missing or not a table\n";
        lua_close(L);
        return entities;
    }

    // Iterate array-style
    lua_pushnil(L);
    while (lua_next(L, -2)) {
        // key at -2, value (entity table) at -1
        if (lua_istable(L, -1)) {
            Entity e;

            // Get id
            lua_getfield(L, -1, "id");
            if (lua_isstring(L, -1)) {
                e.id = lua_tostring(L, -1);
            }
            lua_pop(L, 1);

            // Get type
            lua_getfield(L, -1, "type");
            if (lua_isstring(L, -1)) {
                e.type = lua_tostring(L, -1);
            }
            lua_pop(L, 1);

            // Get position
            lua_getfield(L, -1, "position");
            if (lua_istable(L, -1)) {
                for (int i = 0; i < 3; ++i) {
                    lua_rawgeti(L, -1, i + 1);
                    if (lua_isnumber(L, -1)) {
                        e.position[i] = static_cast<float>(lua_tonumber(L, -1));
                    }
                    lua_pop(L, 1);
                }
            }
            lua_pop(L, 1); // pop position table

            // Get scripts
            lua_getfield(L, -1, "scripts");
            if (lua_istable(L, -1)) {
                int len = lua_rawlen(L, -1);
                for (int i = 1; i <= len; ++i) {
                    lua_rawgeti(L, -1, i);
                    if (lua_isstring(L, -1)) {
                        e.scripts.push_back(lua_tostring(L, -1));
                    }
                    lua_pop(L, 1);
                }
            }
            lua_pop(L, 1); // pop scripts table

            entities.push_back(e);
        }

        lua_pop(L, 1); // pop value, keep key
    }

    return entities;
}