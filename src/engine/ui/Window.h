//
// Created by mgrus on 21.02.2025.
//

#ifndef WINDOW_H
#define WINDOW_H

#include <glm\glm.hpp>

class Window {
public:
    Window(int w, int h, bool fullscreen);
    ~Window();
    void render();


private:
    glm::vec3 location;
    int height = 0;
    int width = 0;
    bool fullscreen = false;
};



#endif //WINDOW_H
