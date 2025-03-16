//
// Created by mgrus on 14.03.2025.
//

#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include <glm/glm.hpp>
#include <vector>

class ComputeShader;
class Mesh;
class Texture;
class Particle;

namespace gru {

  // Define Particle struct matching GLSL (alignment!)
  struct Particle {
    glm::vec4 position;  // xyz position, w unused
    glm::vec4 velocity;  // xyz velocity, w unused
    glm::vec4 emitterPosition; // base emitter position
    float lifetime;
    int type;
    glm::vec2 padding;   // to align size to 16-byte multiples
  };

  // class Particle {
  // public:
  //   glm::vec3 accel;
  //   glm::vec3 location;
  //   glm::vec3 scale;
  //   glm::vec3 rotation;
  //   glm::vec3 direction = {0, 1, 0};
  //   glm::vec4 color = {1, 1, 1, 1};
  //   float lifeTime = 0;
  //   bool remove = false;
  //
  // };

  class ParticleSystem {
  public:
    void initMegaBuffer();

    /**
    * If mesh is a nullptr, a quad will be used.
    */
    ParticleSystem(Mesh* mesh, Texture* texture, glm::vec3 location = {0,0, 0}, int numParticles = 5000, bool useInstancing = true);
    void reset();
    void update();
    void draw(Camera* camera) const;

  private:
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
    Mesh* mesh = nullptr;
    Texture* texture =nullptr;
    std::vector<Particle> particles;

    Shader * particleShader = nullptr;
    ComputeShader* positionComputeShader = nullptr;

    // This holds vertices, uvs, normals, tangents for every particle.
    // UVs and normals don't change per frame, but the position does,
    // so we calculate the world positions on the CPU and update the VBO per frame.
    Mesh * compoundMesh = nullptr;
    int numParticles = 0;

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
}




#endif //PARTICLESYSTEM_H
