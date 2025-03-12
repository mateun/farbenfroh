//
// Created by mgrus on 02.03.2025.
//

#ifndef GAMEPLAYLEVEL_H
#define GAMEPLAYLEVEL_H


class DefaultGame;

namespace ttg {

class GameplayLevel : public GameLevel {

public:

    GameplayLevel(DefaultGame* game, const std::string& name = "gameplay");

    void renderShadowBias();

    void render() override;
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
    SceneNode* flyCamNode = nullptr;
    float shadowBias =0;
    CharacterController * characterController = nullptr;

};

} // ttg

#endif //GAMEPLAYLEVEL_H
