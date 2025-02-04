//
// Created by mgrus on 03.02.2025.
//

#ifndef SERVERVIZ_H
#define SERVERVIZ_H

#include "../../../src/engine/game/default_game.h"

class ServerViz : public DefaultGame {

    void init() override;
    void update() override;
    void render() override;
    bool shouldStillRun() override;
    bool shouldAutoImportAssets() override;
    std::vector<std::string> getAssetFolder() override;


    CameraMover* cameraMover = nullptr;
    FrameBuffer * _fullscreenFrameBuffer = nullptr;
    FrameBuffer * _postProcessedFrameBuffer = nullptr;

    bool fade = false;
    Shader * _postProcessShader = nullptr;
    Camera* _cameraIn3DWorld = nullptr;
};



#endif //SERVERVIZ_H
