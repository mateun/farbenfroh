//
// Created by mgrus on 14.03.2025.
//

#include "ParticleSystem.h"
#include <GL/glew.h>
#include <engine/compute/ComputeShader.h>
#include <engine/game/Timing.h>
#include <engine/graphics/Mesh.h>
#include <engine/graphics/Shader.h>
#include <engine/graphics/PlanePivot.h>
#include <engine/graphics/Geometry.h>
#include <engine/graphics/ErrorHandling.h>
#include <engine/graphics/MeshDrawData.h>
#include <engine/graphics/Renderer.h>

void gru::ParticleEmitter::initMegaBuffer() {
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

void gru::ParticleEmitter::disable() {
    this->active = false;
}

void gru::ParticleEmitter::enable() {
    this->active = true;
}


std::shared_ptr<Shader> gru::ShaderFactory::smokeShader = nullptr;
std::shared_ptr<Shader> gru::ShaderFactory::explosionShader = nullptr;
std::shared_ptr<ComputeShader> gru::ShaderFactory::smokeComputeShader = nullptr;
std::shared_ptr<ComputeShader> gru::ShaderFactory::fireComputeShader = nullptr;
std::shared_ptr<ComputeShader> gru::ShaderFactory::trailComputeShader = nullptr;
std::shared_ptr<ComputeShader> gru::ShaderFactory::explosionComputeShader = nullptr;

std::shared_ptr<Shader> gru::ShaderFactory::getShaderByType(EmitterType type) {
    if (!smokeShader) {
        smokeShader = std::make_shared<Shader>();
        smokeShader->initFromFiles("../assets/shaders/fx/smoke.vert", "../assets/shaders/fx/smoke.frag");
    }
    if (!explosionShader) {
        explosionShader = std::make_shared<Shader>();
        explosionShader->initFromFiles("../assets/shaders/fx/explosion.vert", "../assets/shaders/fx/explosion.frag");
    }
    switch (type) {
        case EmitterType::SMOKE: return smokeShader;
        case EmitterType::EXPLOSION: ; return explosionShader;
        default: return nullptr;
    }

}

std::shared_ptr<ComputeShader> gru::ShaderFactory::getComputeShaderByType(EmitterType type) {
    if (!smokeComputeShader && type == EmitterType::SMOKE) {
        smokeComputeShader = std::make_shared<ComputeShader>("../src/engine/fx/shaders/smoke.comp", std::vector<std::string>{"../src/engine/fx/shaders/helper_funcs.comp"});
    }
    else if (!explosionComputeShader && type == EmitterType::EXPLOSION) {
        explosionComputeShader = std::make_shared<ComputeShader>("../src/engine/fx/shaders/explosion.comp", std::vector<std::string>{"../src/engine/fx/shaders/helper_funcs.comp"});
    }
    else if (!trailComputeShader && type == EmitterType::TRAIL) {
        trailComputeShader = std::make_shared<ComputeShader>("../src/engine/fx/shaders/trail.comp", std::vector<std::string>{"../src/engine/fx/shaders/helper_funcs.comp"});
    }
    else if (!fireComputeShader && type == EmitterType::FIRE) {
        fireComputeShader = std::make_shared<ComputeShader>("../src/engine/fx/shaders/fire.comp", std::vector<std::string>{"../src/engine/fx/shaders/helper_funcs.comp"});
    }
    switch (type) {
        case EmitterType::SMOKE:   return smokeComputeShader;
        case EmitterType::EXPLOSION: return explosionComputeShader;
        case EmitterType::TRAIL: return trailComputeShader;
        case EmitterType::FIRE: return fireComputeShader;
        default: return nullptr;
    }

}


/**
* An emitter needs a mesh, a texture and 3 shaders:
* - compute shader to handle position & velocity updates
* - vertex shader to apply the transformation
* - fragment shader for any color based and transparent effects like fading in and out.
* If no specific mesh is passed in, a quad will be used as the mesh.
*/
gru::ParticleEmitter::ParticleEmitter(const std::shared_ptr<ParticleSystem>& particleSystem, Mesh *mesh, Texture *texture, EmitterType type, glm::vec3 location,
                                      int numParticles, bool useInstancing, bool loop) : mesh(mesh), texture(texture), numParticles(numParticles), location(location), type(type), particleSystem(particleSystem) {
    particleShader = ShaderFactory::getShaderByType(type);
    computeShader = ShaderFactory::getComputeShaderByType(type);

    if (useInstancing) {
        std::vector<int> particleIDs(numParticles);
        for (int i = 0; i < numParticles; ++i)
            particleIDs[i] = i;

        if (!mesh) {
            auto quadMesh = Geometry::createQuadMesh(PlanePivot::center);
            this->mesh = std::move(quadMesh);
        }

        // Attach a new buffer for the particle ids to the given mesh VAO:
        glBindVertexArray(this->mesh->vao);
        GLuint particleID_VBO;
        glGenBuffers(1, &particleID_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, particleID_VBO);
        glBufferData(GL_ARRAY_BUFFER, particleIDs.size() * sizeof(int), particleIDs.data(), GL_STATIC_DRAW);

        // Integer attribute setup (note "I" in glVertexAttribIPointer!)
        glEnableVertexAttribArray(3);
        glVertexAttribIPointer(3, 1, GL_INT, 0, nullptr);
        glVertexAttribDivisor(3, 1);  // per instance
        glBindVertexArray(0);

        // Initialize the particles
        for (int i = 0; i < numParticles; i++) {
            auto p = Particle();
            p.position = glm::vec4(location, 0);
            p.velocity = {2, 0.5, -3, 0};
            p.emitterPosition = glm::vec4(location, 0);
            p.lifetime = float(rand()%1200) / 1000.0f; // random 0–.2 sec;
            //p.lifetime = 0; // to provoke immediate reset in the shader
            p.loop = loop ? 1 : 0;
            particles.push_back(p);
        }

        // Init SSBO for particles
        {
            glGenBuffers(1, &particleSSBO);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleSSBO);
            size_t size = particles.size() * sizeof(Particle);
            glBufferData(GL_SHADER_STORAGE_BUFFER, size, particles.data(), GL_DYNAMIC_COPY);
        }

    } else {
        initMegaBuffer();
    }

}

void gru::ParticleEmitter::reset() {
    done = false;
    initialized = false;
    finishedParticles = 0;
    active = false;
}


void gru::ParticleEmitter::update() {
    if (!active) return;


    // Use a ComputeShader to update all particles
    computeShader->use();
    computeShader->bindSSBO(particleSSBO);
    computeShader->setFloat("deltaTime", Timing::lastFrameTimeInSeconds());
    GL_ERROR_EXIT(556688);
    GLuint numWorkGroups = (numParticles + 255) / 256;
    computeShader->dispatch(DispatchOutput::Buffer, {numWorkGroups, 1, 1});
    GL_ERROR_EXIT(556689);


}


void gru::ParticleEmitter::draw(const Camera* camera) const {
    if (!active) return;

    MeshDrawData mdd;
    mdd.location = location;
    // TODO orientation?!
    mdd.mesh = mesh.get();
    mdd.texture = texture.get();
    mdd.shader = particleShader.get();

    mdd.camera = camera;
    mdd.instanceCount = numParticles;
    computeShader->bindSSBO(particleSSBO);

    glDepthMask(GL_FALSE);
    Renderer::drawMesh(mdd);
    glDepthMask(GL_TRUE);


}

void gru::ParticleSystem::addEmitter(ParticleEmitter *emitter, EmitterExecutionRule emitterRule) {
    emitters.push_back(emitter);
    ruleMap[emitter] = emitterRule;
}

void gru::ParticleSystem::update() {
    timeElapsed += Timing::lastFrameTimeInSeconds();

    // If this is the first time around, we need to check if any of the emitters have a startup delay defined.
    // So we only are allowed to start (update) them if the time is right.
    for (auto e : emitters) {
        auto rule = ruleMap[e];
        if (timeElapsed >= rule.startDelay) {
            e->enable();
            e->update();
        }

        if (rule.maxDuration > 0) {
            if (timeElapsed >= rule.maxDuration) {
                e->disable();
            }
        }
    }



}

void gru::ParticleSystem::render(const Camera *camera) {
    for (auto e : emitters) {
        if (e->isActive()) {
            e->draw(camera);
        }
    }
}

bool gru::ParticleEmitter::isActive() {
    return active;
}