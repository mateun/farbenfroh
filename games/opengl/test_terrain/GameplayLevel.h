//
// Created by mgrus on 02.03.2025.
//

#ifndef GAMEPLAYLEVEL_H
#define GAMEPLAYLEVEL_H


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
    SceneNode * terrainNode = nullptr;
    SceneNode * shotCursorNode = nullptr;
    Shader * basicShader = nullptr;
    CameraMover * cameraMover = nullptr;
    Camera* debugFlyCam = nullptr;
    CameraMover* flyCamMover = nullptr;
    bool inFlyCamDebugMode = false;
    SceneNode * cameraNode = nullptr;
    SceneNode* heroNode = nullptr;
    SceneNode * padNode = nullptr;
    SceneNode* flyCamNode = nullptr;
    float shadowBias =0;
    CharacterController * characterController = nullptr;
    MeshDrawData * playerBulletMeshData = nullptr;
    std::vector<SceneNode*> playerBulletPool;
    std::vector<SceneNode*> enemyList;
};

} // ttg

#endif //GAMEPLAYLEVEL_H
