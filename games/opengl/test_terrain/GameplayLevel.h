//
// Created by mgrus on 02.03.2025.
//

#ifndef GAMEPLAYLEVEL_H
#define GAMEPLAYLEVEL_H


class EnemyExplosionComponent;
class DefaultGame;

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

    void renderShadowBias();

    void renderPlayerStats();

    void render() override;

    SceneNode* findFirstInactive(const std::vector<SceneNode *> & nodeList);

    void updatePlayerBullets();

    void checkPlayerCollision();

    void cameraUpdate();

    void update() override;
    void init() override;


    Terrain * terrain = nullptr;
    Scene * scene = nullptr;

    Shader * basicShader = nullptr;
    Shader * basicShaderUnlit = nullptr;
    Shader * emissiveShader = nullptr;
    CameraMover * cameraMover = nullptr;
    Camera* debugFlyCam = nullptr;
    CameraMover* flyCamMover = nullptr;
    bool inFlyCamDebugMode = false;



    float shadowBias =0;
    CharacterController * characterController = nullptr;
    MeshDrawData * playerBulletMeshData = nullptr;
    std::vector<SceneNode*> playerBulletPool;

    gru::ParticleEmitter * peSmoke0 = nullptr;
    gru::ParticleEmitter * peExplosion0 = nullptr;
    gru::ParticleEmitter * peSmoke2 = nullptr;

    gru::ParticleSystem * psystem0 = nullptr;

};

} // ttg

#endif //GAMEPLAYLEVEL_H
