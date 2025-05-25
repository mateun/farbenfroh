//
// Created by mgrus on 25.05.2025.
//

#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <string>
#include <array>

struct Entity {
    std::string id;
    std::string type;
    std::array<float, 3> position = {0.0f, 0.0f, 0.0f};
    std::vector<std::string> scripts;
};

struct UIElement {
    std::string id;
    std::string type;
    std::string text;
    std::array<float, 2> position = {0.0f, 0.0f};
    std::array<float, 2> size = {0.0f, 0.0f};
    std::string anchor = "top_left";
    int font_size = 12;
};


#endif //GAMESTATE_H
