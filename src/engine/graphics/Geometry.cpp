//
// Created by mgrus on 25.03.2025.
//

#include "Geometry.h"

#include "ErrorHandling.h"


GLuint gru::Geometry::genVec3Buffer(int index,  std::vector<float> data) {
    GLuint buf;
    glGenBuffers(1, &buf);
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(index);
    return buf;

}

GLuint gru::Geometry::genVec3Buffer(int index,  std::vector<glm::vec3> data) {
    GLuint buf;
    glGenBuffers(1, &buf);
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(glm::vec3), data.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(index);
    return buf;

}

GLuint gru::Geometry::genVec2Buffer(int index,  std::vector<glm::vec2> data) {
    GLuint buf;
    glGenBuffers(1, &buf);
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    auto raw = data.data();
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(glm::vec2), raw, GL_STATIC_DRAW);
    glVertexAttribPointer(index, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(index);
    return buf;
}

GLuint gru::Geometry::genIndexBuffer(std::vector<GLuint> indices) {
    GLuint buf;
    glGenBuffers(1, &buf);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(), GL_STATIC_DRAW);
    return buf;

}

GLuint gru::Geometry::createAndBindVAO() {
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    return vao;
}

void gru::Geometry::unbindVAO() {
    glBindVertexArray(0);
}


std::unique_ptr<Mesh> gru::Geometry::createQuadMesh(PlanePivot pivot) {
    auto mesh = std::make_unique<Mesh>();
    std::vector<glm::vec3> basePositions = {
        {-0.5, 0.5, 0},      // tl
    {-0.5, -0.5, 0},        // bl
    {0.5, -0.5, 0},         // br
        {0.5, 0.5, 0}};     // tr

    glm::vec3 pivotOffset = {0, 0, 0};
    if (pivot == PlanePivot::bottomleft) {
        pivotOffset = {0.5, 0.5, 0};
    }
    else if (pivot == PlanePivot::topleft) {
        pivotOffset = {0.5, -0.5, 0};
    }
    else if (pivot == PlanePivot::bottomright) {
        pivotOffset = {-0.5, 0.5, 0};
    }
    else if (pivot == PlanePivot::topright) {
        pivotOffset = {-0.5, -0.5, 0};
    }

    // Apply pivot offset
    {
        int i= 0;
        std::vector<glm::vec3> manipulated;
        for (auto bp: basePositions) {
            bp += pivotOffset;
            manipulated.push_back(bp);
            i++;

        }
        basePositions.clear();
        for (auto mp : manipulated) {
            basePositions.push_back(mp);
        }
    }

    std::vector<uint32_t> indices = {
        0, 1, 2,
        0, 2, 3
    };

    std::vector<glm::vec2> uvs = {
        {0, 1},
        {0, 0},
        {1, 0},
        {1, 1},

    };

    std::vector<float> posFlat;
    for (auto bp : basePositions) {
        posFlat.push_back(bp.x);
        posFlat.push_back(bp.y);
        posFlat.push_back(bp.z);
    }

    auto vao = createAndBindVAO();
    genVec3Buffer(0, basePositions);
    genVec2Buffer(1, uvs);
    genIndexBuffer(indices);
    GL_ERROR_EXIT(8123);
    unbindVAO();

    mesh->vao = vao;
    mesh->positions = basePositions;
    mesh->indices = indices;
    return mesh;

}

std::unique_ptr<int> gru::Geometry::createQMesh() {
    return std::make_unique<int>(534);
}

