//
// Created by mgrus on 25.03.2025.
//

#ifndef MESH_H
#define MESH_H

#include <GL\glew.h>
#include <vector>
#include <string>
#include <memory>
#include <glm\glm.hpp>
#include <engine/animation/Skeleton.h>
#include <engine/animation/Animation.h>

#include <ozz/animation/runtime/skeleton.h>

class Ray;
class Centroid;

/**
 * A 3D mesh which can be rendered anywhere
 * in the world.
 * Bind it and then it can be rendered.
 */
class Mesh {
public:
    GLuint vao = 0;
    GLuint instanceOffsetVBO = 0;
    GLuint instanceMatrixVBO = 0;
    GLuint instanceColorVBO = 0;
    GLuint instanceTintVBO = 0;
    int numberOfIndices = -1;
    GLenum indexDataType = GL_UNSIGNED_INT;

    gru::Skeleton* skeleton = nullptr;
    std::shared_ptr<ozz::animation::Skeleton> ozzSkeleton;

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> positionsSortedByIndex;
    std::vector<uint32_t> indices;
    std::vector<glm::vec3> tangents;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    std::vector<glm::ivec4> boneIndices;
    std::vector<glm::vec4> boneWeights;
    std::string fileName;

    std::vector<Animation*> animations;
    GLuint positionVBO;
    GLuint indicesVBO;
    GLuint uvsVBO;
    GLuint normalsVBO;

    bool rayCollides(Ray ray, glm::vec4& color);
    Animation* findAnimation(const std::string& name);

    std::vector<Centroid*> calculateCentroids();

    std::vector<Centroid*> centroids;
};



#endif //MESH_H
