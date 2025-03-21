//
// Created by mgrus on 02.03.2025.
//

#ifndef GAMEPLAYLEVEL_H
#define GAMEPLAYLEVEL_H

class CollisionManager;
class PlayerBulletPool;
class EnemyExplosionComponent;
class DefaultGame;
class PlayerShooting;

namespace ttg {

    class BulletData {
    public:
        float maxLifeInSeconds = 2.0f;
        float currentLifeInSeconds = 0.0f;
        int damage = 100;
        int bouncesLeft = 0;
    };

class GameplayLevel : public GameLevel {

public:

    GameplayLevel(DefaultGame* game, const std::string& name = "gameplay");

    void init() override;
    void update() override;
    void cameraUpdate();

    void render() override;
    void renderShadowBias();
    void renderPlayerStats();
    void updatePlayerBullets();
    void checkPlayerCollision();

    Terrain * terrain = nullptr;
    Scene * scene = nullptr;

    std::unique_ptr<Shader> basicShader = nullptr;
    Shader * basicShaderUnlit = nullptr;
    Shader * emissiveShader = nullptr;
    CameraMover * cameraMover = nullptr;
    Camera* debugFlyCam = nullptr;
    CameraMover* flyCamMover = nullptr;
    bool inFlyCamDebugMode = false;

    float shadowBias =0;
    CharacterController * characterController = nullptr;
    MeshDrawData * playerBulletMeshData = nullptr;

    gru::ParticleEmitter * peSmoke0 = nullptr;
    gru::ParticleEmitter * peExplosion0 = nullptr;
    gru::ParticleEmitter * peSmoke2 = nullptr;

    gru::ParticleSystem * psystem0 = nullptr;

private:
    std::shared_ptr<SceneNode> padNode;
    std::shared_ptr<SceneNode> cameraNode;
    std::unique_ptr<Mesh> spawnDecalQuadMesh;

    std::unique_ptr<PlayerShooting> playerShootingLogic;
    std::shared_ptr<PlayerBulletPool> playerBulletPool;
    std::unique_ptr<CollisionManager> collisionManager;
    std::vector<std::shared_ptr<SceneNode>> enemies;

};

} // ttg

#endif //GAMEPLAYLEVEL_H
