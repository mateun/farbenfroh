//
// Created by mgrus on 02.03.2025.
//

#ifndef GAMEPLAYLEVEL_H
#define GAMEPLAYLEVEL_H


class DefaultGame;

namespace ttg {

class GameplayLevel : public GameLevel {

public:

    GameplayLevel(DefaultGame* game);
    void render() override;
    void update() override;
    void init() override;


    Terrain * terrain = nullptr;
    Scene * scene = nullptr;
    SceneNode * terrainNode = nullptr;
    Shader * basicShader = nullptr;
    CameraMover * cameraMover = nullptr;
    Camera* debugFlyCam = nullptr;
    CameraMover* flyCamMover = nullptr;
    bool inFlyCamDebugMode = false;
    SceneNode * cameraNode = nullptr;
    SceneNode* flyCamNode = nullptr;
};

} // ttg

#endif //GAMEPLAYLEVEL_H
