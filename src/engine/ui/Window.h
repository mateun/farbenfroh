//
// Created by mgrus on 21.02.2025.
//

#ifndef WINDOW_H
#define WINDOW_H

#include <glm\glm.hpp>

class Window {
public:
    Window();
    ~Window();
    void render();


private:
    glm::vec3 location;

};



#endif //WINDOW_H
