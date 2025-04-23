//
// Implements everything related to terrains
//
// Created by mgrus on 19.10.2024.
//
#include "terrain.h"

#include <engine/graphics/Renderer.h>
#include <engine/graphics/StatefulRenderer.h>
#include <GL/glew.h>

#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_access.hpp>


Terrain::Terrain(int horizontalCells, int verticalCells, glm::mat4* rotation, float sphereMapRadius) {

    this->rotationMatrix = rotation;
    this->sphereMapRadius = sphereMapRadius;
    int columns = horizontalCells;
    int rows = verticalCells;

    GLuint vboPos;
    std::vector<float> positions;


    GLuint vboIndices;
    std::vector<uint32_t> indices;

    GLuint vboUVs;
    std::vector<float> uvs;


    GLuint vboNormals;
    std::vector<float> normals;

    GLuint vboTangents;

    std::vector<float> tangents;


    int posCounter = 0;
    int indexCounter = 0;
    int uvCounter = 0;
    int normalCounter = 0;
    int vertexCounter = 0;
    int colCounter = 0;
    int rowCounter = 0;

    int leftColumn = (columns / 2) * -1;
    int rightColumn = leftColumn + columns;
    int topRow = (rows/2) * -1;
    int bottomRow = topRow + rows;

    for (float r = topRow; r < bottomRow; r++) {
        colCounter = 0;
        for (float i = leftColumn; i < rightColumn; i++) {

            auto heightValueTL = getHeightValueAt(i, r, CellType::TopLeft);
            auto heightValueTR = getHeightValueAt(i, r, CellType::TopRight);
            auto heightValueBL = getHeightValueAt(i, r, CellType::BottomLeft);
            auto heightValueBR = getHeightValueAt(i, r, CellType::BottomRight);

            // Seen from above:
            // top left
            glm::vec3 posTL = glm::vec3(i, heightValueTL, r);
            applyRotationMatrix(posTL);
            mapPosToSphere(posTL);
            positions.push_back(posTL.x);
            positions.push_back(posTL.y);
            positions.push_back(posTL.z);

            // top right
            glm::vec3 posTR = glm::vec3(i+1, heightValueTR, r);
            applyRotationMatrix(posTR);
            mapPosToSphere(posTR);
            positions.push_back(posTR.x);
            positions.push_back(posTR.y);
            positions.push_back(posTR.z);

            // bottom left
            glm::vec3 posBL = glm::vec3(i, heightValueBL, r+1);
            applyRotationMatrix(posBL);
            mapPosToSphere(posBL);
            positions.push_back(posBL.x);
            positions.push_back(posBL.y);
            positions.push_back(posBL.z);

            // bottom right
            glm::vec3 posBR = glm::vec3(i+1, heightValueBR, r+1);
            applyRotationMatrix(posBR);
            mapPosToSphere(posBR);
            positions.push_back(posBR.x);
            positions.push_back(posBR.y);
            positions.push_back(posBR.z );
            posCounter += 12;

            // Indices
            indices.push_back(vertexCounter);
            indices.push_back(vertexCounter+2);
            indices.push_back(vertexCounter+1);
            indices.push_back(vertexCounter+1);
            indices.push_back(vertexCounter+2);
            indices.push_back(vertexCounter+3);
            indexCounter += 6;
            vertexCounter += 4;

            // UVs
            float uvStepHorizontal = (float) 1 / (horizontalCells);
            float uvStepVertical = (float)  1 / (verticalCells);
            uvs.push_back(colCounter * uvStepHorizontal);
            uvs.push_back(1- (rowCounter *uvStepVertical));
            uvs.push_back(colCounter * uvStepHorizontal + uvStepHorizontal);
            uvs.push_back(1 - (rowCounter    * uvStepVertical));
            uvs.push_back(colCounter * uvStepHorizontal);
            uvs.push_back(1 - (rowCounter * uvStepVertical + uvStepVertical));
            uvs.push_back(colCounter * uvStepHorizontal + uvStepHorizontal);
            uvs.push_back(1 - (rowCounter * uvStepVertical + uvStepVertical));
            uvCounter += 8;

            colCounter++;
        }
        rowCounter++;
    }

    // Second pass for the normals
    posCounter = 0;
    for (float r = 0; r < rows-1; r++) {
        for (float i = 0; i < columns - 1; i++) {

            posCounter += 12;

            // Calculated normals
            auto posTL = glm::vec3(positions[posCounter-12], positions[posCounter-11], positions[posCounter-10]);
            auto posTR = glm::vec3(positions[posCounter-9], positions[posCounter-8], positions[posCounter-7]);
            auto posBL = glm::vec3(positions[posCounter-6], positions[posCounter-5], positions[posCounter-4]);
            auto posBR = glm::vec3(positions[posCounter-3], positions[posCounter-2], positions[posCounter-1]);

//            auto posTR_Right = i < (columns-2) ? glm::vec3(positions[posCounter-9+12], positions[posCounter-8+12], positions[posCounter-7+12]) : glm::vec3(0, 1,0);
//            auto posBR_Right = i < (columns -2) ?  glm::vec3(positions[posCounter-3 + 12], positions[posCounter-2+12], positions[posCounter-1+12]): glm::vec3{0,1,0};

            auto normTL = glm::normalize(glm::cross(posBL - posTL, posTR - posTL))  ;
            auto normTR = glm::normalize(glm::cross(posTL - posTR, posBR - posTR)) ;
            auto normBL = glm::normalize(glm::cross(posBR - posBL, posTL - posBL)) ;
            auto normBR = glm::normalize(glm::cross(posTR - posBR, posBL - posBR ) );
//            auto normTR_R  = glm::normalize(glm::cross(posBR - posTR, posTR_Right - posTR));
//            auto normTR_R_n = glm::normalize(normTR + normTR_R);

            if (i == 0) {
                normTL = glm::vec3{0, 1, 0};
            }

            normals.push_back(normTL.x);
            normals.push_back(normTL.y);
            normals.push_back(normTL.z);

            normals.push_back(normTR.x);
            normals.push_back(normTR.y);
            normals.push_back(normTR.z);

            normals.push_back(normBL.x);
            normals.push_back(normBL.y);
            normals.push_back(normBL.z);

            normals.push_back(normBR.x);
            normals.push_back(normBR.y);
            normals.push_back(normBR.z);
            normalCounter += 12;

            // Create tangents as well
            // TODO calculate for real
            glm::vec3 tangentTemp = {1, 1, 0};

            tangents.push_back(tangentTemp.x);
            tangents.push_back(tangentTemp.y);
            tangents.push_back(tangentTemp.z);

            tangents.push_back(tangentTemp.x);
            tangents.push_back(tangentTemp.y);
            tangents.push_back(tangentTemp.z);

            tangents.push_back(tangentTemp.x);
            tangents.push_back(tangentTemp.y);
            tangents.push_back(tangentTemp.z);

            tangents.push_back(tangentTemp.x);
            tangents.push_back(tangentTemp.y);
            tangents.push_back(tangentTemp.z);

        }
    }

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);


    glGenBuffers(1, &vboPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * 4, positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    GLuint vbUvs;
    glGenBuffers(1, &vbUvs);
    glBindBuffer(GL_ARRAY_BUFFER, vbUvs);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * 4, uvs.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);


    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * 4, normals.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    glGenBuffers(1, &vboTangents);
    glBindBuffer(GL_ARRAY_BUFFER, vboTangents);
    glBufferData(GL_ARRAY_BUFFER, tangents.size() * 4, tangents.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(12, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(12);

    unsigned int instanceVBO;
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 100, nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glVertexAttribDivisor(3, 1);


    glGenBuffers(1, &vboIndices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * 4, indices.data(), GL_DYNAMIC_DRAW);

    terrainMesh = new Mesh();
    terrainMesh->vao = vao;
    terrainMesh->numberOfIndices = indices.size();
    terrainMesh->indexDataType  = GL_UNSIGNED_INT;
    terrainMesh->indices = indices;
    terrainMesh->instanceOffsetVBO = instanceVBO;

}

float Terrain::getHeightValueAt(float x, float y, CellType cellType) {
    return sphereMapRadius;
}


// Make this position be radius length away from the origin 0/0.
// This allows for spherical mapping.
void Terrain::mapPosToSphere(glm::vec3& posVertex) {
    if (sphereMapRadius == 0) {
        return;
    }
    posVertex = glm::normalize(posVertex) * sphereMapRadius;

}

void Terrain::render() {
    StatefulRenderer::bindMesh(terrainMesh);
    StatefulRenderer::drawMesh();

}

Mesh * Terrain::getMesh() {
    return terrainMesh;
}

void Terrain::applyRotationMatrix(glm::vec3& vertex) {
    if (rotationMatrix) {
        auto transformedVertex = (*rotationMatrix) * glm::vec4(vertex, 1);
        vertex.x = transformedVertex.x;
        vertex.y = transformedVertex.y;
        vertex.z = transformedVertex.z;
    }

}

