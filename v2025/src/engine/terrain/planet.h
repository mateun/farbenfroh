//
// Created by mgrus on 19.10.2024.
//

#ifndef SIMPLE_KING_PLANET_H
#define SIMPLE_KING_PLANET_H
#include "terrain.h"


namespace fireball {

    /**
 * A sphere consists of a cube of terrain planes,
 * which are projected on the (planet's) sphere.
 *
 */
    class Sphere {

    public:
        Sphere(int radius, glm::vec3 location);
        void render();
        float getRadius();
        glm::vec3 getWorldLocation();
        void setWorldLocation(glm::vec3 loc);

    private:
        Terrain *terrainTop;
        Terrain *terrainBottom;
        Terrain *terrainRight;
        Terrain *terrainLeft;
        Terrain *terrainFront;
        Terrain *terrainBack;

        float radius;
        glm::vec3 worldLocation;

    };


}


#endif //SIMPLE_KING_PLANET_H
