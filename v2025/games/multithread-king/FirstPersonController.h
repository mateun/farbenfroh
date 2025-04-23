//
// Created by mgrus on 29.05.2024.
//

#ifndef SIMPLE_KING_FIRSTPERSONCONTROLLER_H
#define SIMPLE_KING_FIRSTPERSONCONTROLLER_H

#include <glm/glm.hpp>

namespace mtking {

    class Character{
    public:
        glm::vec3 position;
    };

    class FirstPersonController {
    public:
        FirstPersonController(Character* character, Camera* camera);
        void updateMovement();

    private:
        Character* _character = nullptr;
        Camera* _camera = nullptr;
    };

} // mtking

#endif //SIMPLE_KING_FIRSTPERSONCONTROLLER_H
