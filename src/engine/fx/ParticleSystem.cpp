//
// Created by mgrus on 14.03.2025.
//

#include "ParticleSystem.h"
#include <engine/compute/ComputeShader.h>
#include "engine/math/math3d.h"


void gru::ParticleSystem::initMegaBuffer() {
        // Now we put all the vertices of all instances into a single VBO
    // to render the complete particle system in one go.
    {

        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        // We take the positions of our base mesh and duplicate them into
        // single buffer.

        static float iZ = 10;
        for (int i = 0; i< numParticles; i++) {
            if (i % 10 == 0) {
                iZ -= 3;
            }
            int count = 0;
            for (auto basePosition : mesh->positions) {
                    auto ipos = basePosition;

                    float iX = -20 + ((i % 10) * 4);
                    ipos += glm::vec3(iX, 0, iZ);
                    instancedPositions.push_back(ipos);

                    auto inormal = mesh->normals[count];
                    instancedNormals.push_back(inormal);

                    auto iuv = mesh->uvs[count];
                    instancedUVs.push_back(iuv);
            }
            count++;
        }


        for (int i = 0; i< numParticles; i++) {
            for (auto baseIndex : mesh->indices) {
                auto iindex = baseIndex;
                iindex = iindex + (mesh->positions.size() * i);
                instancedIndices.push_back(iindex);
            }
        }



        std::vector<float> posFlat;
        for (auto p : instancedPositions) {
            posFlat.push_back(p.x);
            posFlat.push_back(p.y);
            posFlat.push_back(p.z);
        }

        std::vector<float> uvsFlat;
        for (auto p : instancedUVs) {
            uvsFlat.push_back(p.x);
            uvsFlat.push_back(p.y);
        }

        std::vector<float> normalsFlat;
        for (auto p : instancedNormals) {
            normalsFlat.push_back(p.x);
            normalsFlat.push_back(p.y);
            normalsFlat.push_back(p.z);
        }


        GLuint vboPos;
        glGenBuffers(1, &vboPos);
        glBindBuffer(GL_ARRAY_BUFFER, vboPos);
        glBufferData(GL_ARRAY_BUFFER, posFlat.size() * 4, posFlat.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        GLuint vbUvs;
        glGenBuffers(1, &vbUvs);
        glBindBuffer(GL_ARRAY_BUFFER, vbUvs);
        glBufferData(GL_ARRAY_BUFFER, uvsFlat.size() * 4, uvsFlat.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);

        GLuint vboNormals;
        glGenBuffers(1, &vboNormals);
        glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
        glBufferData(GL_ARRAY_BUFFER, normalsFlat.size() * 4, normalsFlat.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(2);

        GLuint vboIndices;
        glGenBuffers(1, &vboIndices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, instancedIndices.size() * 4, instancedIndices.data(), GL_DYNAMIC_DRAW);

        glBindVertexArray(0);


        compoundMesh = new Mesh();
        compoundMesh->vao = vao;
        compoundMesh->uvs = instancedUVs;
        compoundMesh->normals = instancedNormals;
        compoundMesh->indices = instancedIndices;
        compoundMesh->positions = instancedPositions;
        compoundMesh->positionVBO = vboPos;
        compoundMesh->uvsVBO = vbUvs;
        compoundMesh->normalsVBO = vboNormals;
        compoundMesh->indicesVBO = vboIndices;
    }

}

gru::ParticleSystem::ParticleSystem(Mesh *mesh, Texture *texture, glm::vec3 location, int numParticles, bool useInstancing) : mesh(mesh), texture(texture), numParticles(numParticles), location(location) {
    particleShader = new Shader();
    particleShader->initFromFiles("../assets/shaders/particle.vert", "../assets/shaders/particle.frag");
    positionComputeShader = new ComputeShader("../src/engine/fx/shaders/particles.comp");

    if (useInstancing) {
        std::vector<int> particleIDs(numParticles);
        for (int i = 0; i < numParticles; ++i)
            particleIDs[i] = i;

        glBindVertexArray(mesh->vao);
        GLuint particleID_VBO;
        glGenBuffers(1, &particleID_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, particleID_VBO);
        glBufferData(GL_ARRAY_BUFFER, particleIDs.size() * sizeof(int), particleIDs.data(), GL_STATIC_DRAW);

        // Integer attribute setup (note "I" in glVertexAttribIPointer!)
        glEnableVertexAttribArray(3);
        glVertexAttribIPointer(3, 1, GL_INT, 0, nullptr);
        glVertexAttribDivisor(3, 1);  // per instance

        for (int i = 0; i < numParticles; i++) {
            auto p = Particle();
            p.position = {0, 0, 0, 0};
            p.velocity = {0, 0, 0, 0};
            p.emitterPosition = glm::vec4(location, 0);
            p.lifetime = float(rand()%5000) / 1000.0f; // random 0–5 sec;
            p.type = 0;
            particles.push_back(p);


        }
        glBindVertexArray(0);
        positionComputeShader->initWithShaderStorageBuffer(particles);

    } else {
        initMegaBuffer();
    }



}

void gru::ParticleSystem::reset() {
    done = false;
    initialized = false;
    finishedParticles = 0;
}


void gru::ParticleSystem::update() {
    static float timePassed = 0.0f;
    timePassed += ftSeconds * 0.5f;

    // Temp
    // Use CPU to compute new particle positions:
    // std::vector<int> indices;
    // std::vector<glm::vec3> positions;
    // static float timePassed = 0.0f;
    // timePassed += ftSecs * 0.5f;
    // for (int i= 0; i< 1000; i++) {
    //     // Generate a new random position offset
    //     float rx = randomFloat(-5, 5);
    //     float rz = randomFloat(-5, 5);
    //     float freq = 2.0f;
    //     float phaseOffset = i * glm::pi<float>() * 0.3f;
    //     float y = sinf((timePassed * freq) + phaseOffset);
    //
    //     indices.push_back(i);
    //     positions.push_back({y, 0, 0});
    // }
    //updateParticlePositions(indices, positions, false);

    // end temp

    // Computeshader to update all particles
    // Bind and dispatch compute shader
    positionComputeShader->setFloat("deltaTime", timePassed);
    GLuint numWorkGroups = (numParticles + 255) / 256;
    positionComputeShader->dispatch(DispatchOutput::Buffer, {numWorkGroups, 1, 1});


    // Memory barrier (ensures particle data is visible after compute shader)




}

void gru::ParticleSystem::updateParticlePositions(std::vector<int> indices, std::vector<glm::vec3> positions, bool accumulative) {

    int numberOfPositions = mesh->positions.size();
    std::vector<float> posFlat;
    for (auto index : indices) {
        // How many vertices do we have per particle (mesh)?
        // Now to reach the correct positions we multiply with the index:
        int positionStartIndex = numberOfPositions * index;
        for (int i = positionStartIndex; i < numberOfPositions + positionStartIndex; i++) {
            glm::vec3 newPos = compoundMesh->positions[i] + positions[index];
            if (accumulative) {
                newPos = compoundMesh->positions[i] += positions[index];
            }

            posFlat.push_back(newPos.x);
            posFlat.push_back(newPos.y);
            posFlat.push_back(newPos.z);
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, compoundMesh->positionVBO);
    glBufferSubData(GL_ARRAY_BUFFER, indices[0] * 3 * sizeof(float), posFlat.size()* sizeof(float), posFlat.data());

}

void gru::ParticleSystem::updateParticlePosition(int particleIndex, glm::vec3 particlePosition, bool accumulative) {
    // How many vertices do we have per particle (mesh)?
    int numberOfPositions = mesh->positions.size();
    // Now to reach the correct positions we multiply with the index:
    int positionStartIndex = numberOfPositions * particleIndex;
    std::vector<float> posFlat;
    for (int i = positionStartIndex; i < numberOfPositions + positionStartIndex; i++) {
        glm::vec3 newPos = compoundMesh->positions[i] + particlePosition;
        if (accumulative) {
            newPos = compoundMesh->positions[i] += particlePosition;
        }

        posFlat.push_back(newPos.x);
        posFlat.push_back(newPos.y);
        posFlat.push_back(newPos.z);
    }

    glBindBuffer(GL_ARRAY_BUFFER, compoundMesh->positionVBO);
    glBufferSubData(GL_ARRAY_BUFFER, positionStartIndex * 3 * sizeof(float), numberOfPositions * 3 * sizeof(float), posFlat.data());

}

void gru::ParticleSystem::draw(Camera* camera) const {
    MeshDrawData mdd;
    mdd.mesh = mesh;
    mdd.texture = texture;
    mdd.shader = particleShader;
    mdd.location = {0, 1, -10};
    mdd.scale = glm::vec3(.2f, .2f, .2f);
    mdd.camera = camera;
    mdd.instanceCount = numParticles;
    drawMesh(mdd);


}


