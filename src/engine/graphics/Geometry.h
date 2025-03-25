//
// Created by mgrus on 25.03.2025.
//

#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <memory>
#include <engine/graphics/Mesh.h>
#include <engine/graphics/PlanePivot.h>

class Geometry {

    public:
    static GLuint genVec3Buffer(int index, std::vector<float> data);
    static GLuint genVec3Buffer(int index,  std::vector<glm::vec3> data);
    static GLuint genVec2Buffer(int index,  std::vector<glm::vec2> data);
    static GLuint genIndexBuffer(std::vector<GLuint> indices);

    static GLuint createAndBindVAO();

    static void unbindVAO();

    static std::unique_ptr<Mesh> createQuadMesh(PlanePivot pivot);

};



#endif //GEOMETRY_H
