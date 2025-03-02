//
// Created by mgrus on 19.10.2024.
//

#ifndef SIMPLE_KING_TERRAIN_H
#define SIMPLE_KING_TERRAIN_H
#include "../../graphics.h"
#include <GL/glew.h>

// Essentially a plane with a variable number of cells inside it.
class Terrain {

public:
    /**
     *
     * @param horizontalCells   How many cells of size 1 do we have in the x direction when viewed top down?
     * @param verticalCells     How many cells of size 1 do we have in the z direction when viewed top down?
     * @param sphereMapRadius   If this terrain plane shall be projected onto a sphere, what is the radius of this sphere?
     *                          Default is 0 which means "no sphere mapping".
     *                          The center of the sphere is always 0/0, so that everything can be moved nicely.
     */
    Terrain(int horizontalCells, int verticalCells, glm::mat4 *rotationMatrix = nullptr, float sphereMapRadius = 0);
    void render();

    enum class CellType {
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
    };

    Mesh* getMesh();

private:
    float getHeightValueAt(float x, float y, CellType cellType);


    GLuint vao;
    glm::mat4* rotationMatrix = nullptr;
    float sphereMapRadius = 0;
    Mesh* terrainMesh = nullptr;

    void mapPosToSphere(glm::vec3& pos);
    void applyRotationMatrix(glm::vec3& vertex);
};


#endif //SIMPLE_KING_TERRAIN_H
