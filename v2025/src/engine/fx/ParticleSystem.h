//
// Created by mgrus on 14.03.2025.
//

#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include <glm/glm.hpp>
#include <GL/glew.h>
#include <vector>
#include <map>
#include <memory>
#include <engine/graphics/Texture.h>

class Camera;
struct Shader;

namespace gru {
  class ParticleSystem;
}

struct MeshDrawData;
class ComputeShader;
class Mesh;

class Particle;

namespace gru {

  // Define Particle struct matching GLSL (alignment!)
  struct Particle {
    glm::vec4 position;  // xyz position, w unused
    glm::vec4 velocity;  // xyz velocity, w unused
    glm::vec4 emitterPosition; // base emitter position
    float lifetime;
    int type;
    int loop;
    int padding;   // to align size to 16-byte multiples
  };

  enum class EmitterType {
    SMOKE,
    FIRE,
    EXPLOSION,
    TRAIL
  };

  class ShaderFactory {
  public:
    static std::shared_ptr<Shader> getShaderByType(EmitterType type);
    static std::shared_ptr<ComputeShader> getComputeShaderByType(EmitterType type);

  private:
    static std::shared_ptr<Shader> smokeShader;
    static std::shared_ptr<Shader> explosionShader;
    static std::shared_ptr<ComputeShader> smokeComputeShader;
    static std::shared_ptr<ComputeShader> fireComputeShader;
    static std::shared_ptr<ComputeShader> trailComputeShader;
    static std::shared_ptr<ComputeShader> explosionComputeShader;


  };

  class ParticleEmitter {
  public:
    void initMegaBuffer();

    void disable();
    void enable();

    bool isActive();


    /**
    * If mesh is a nullptr, a quad will be used.
    */
    ParticleEmitter(const std::shared_ptr<ParticleSystem>& particleSystem, Mesh* mesh, Texture* texture, EmitterType type, glm::vec3 location = {0,0, 0}, int numParticles = 5000, bool useInstancing = true, bool loop = false);
    void reset();
    void update();
    void draw(const Camera* camera) const;


  private:
    std::weak_ptr<ParticleSystem> particleSystem;
    bool initialized = false;
    glm::vec3 location = {0, 0, 0};
    glm::vec3 rotation = {0, 0, 0};
    int number = 1;
    int finishedParticles = 0;
    float scaleMin = 0;
    float scaleMax = 5;
    float maxLifeInSeconds = 2;
    float lifeTimeInSeconds = 0;
    float spawnLag = 0.1;
    float lastSpawnTime = 0;
    float spawnRatePerSecond = 10;
    bool useGravity = true;
    bool oneShot = true;
    bool done = false;
    float gravityValue = -9.81f;
    float initialSpeed = 1;
    std::shared_ptr<Mesh> mesh = nullptr;
    std::shared_ptr<Texture> texture =nullptr;
    std::vector<Particle> particles;

    std::shared_ptr<Shader> particleShader = nullptr;
    std::shared_ptr<ComputeShader> computeShader = nullptr;
    GLuint particleSSBO;

    // This holds vertices, uvs, normals, tangents for every particle.
    // UVs and normals don't change per frame, but the position does,
    // so we calculate the world positions on the CPU and update the VBO per frame.
    Mesh * compoundMesh = nullptr;
    int numParticles = 0;
    bool active = false;
    EmitterType type;

    /**
    * Updates the respective position vertex buffer for the given particle (by its index)
    * to the new (world) position.
    */
    void updateParticlePosition(int particleIndex, glm::vec3 particlePosition, bool accumulative);
    void updateParticlePositions(std::vector<int> indices, std::vector<glm::vec3> positions, bool accumulative);

    std::vector<glm::vec3> instancedPositions;
    std::vector<glm::vec3> instancedNormals;
    std::vector<glm::vec2> instancedUVs;
    std::vector<uint32_t> instancedIndices;
  };

  /**
  * Describes how an emitter should be executed within
  * a particle system. This ties an emitter to a given system,
  * so the emitter can be reused and parameterized for different
  * systems with this rule set.
  */
  class EmitterExecutionRule {
  public:
    float startDelay = 0;
    glm::vec3 locationOffset;
    bool loop = false;
    float maxDuration = 1;
  };

  /**
  * A ParticleSystem contains n ParticleEmitters and
  * coordinates the running of these emitters.
  * These might run the same time or delayed etc.
  * One emitter can be reused with different parameters
  * in different systems.
  */
  class ParticleSystem {
  public:
    void addEmitter(ParticleEmitter* emitter, EmitterExecutionRule emitterRule);
    void update();
    void render(const Camera* camera);

  private:
    std::vector<ParticleEmitter*> emitters;
    std::map<ParticleEmitter*, EmitterExecutionRule> ruleMap;
    bool initialized = false;
    float timeElapsed = 0;
  };
}




#endif //PARTICLESYSTEM_H
