//
// Created by mgrus on 13.02.2025.
//

#include "TestGame1.h"


DefaultApp* getGame() {
    return new TestGame1();
}

void TestGame1::init() {
    DefaultApp::init();
    mechShader = new Shader();
    mechShader->initFromFiles("../games/opengl/testgame1/assets/shaders/mech.vert", "../games/opengl/testgame1/assets/shaders/mech.frag");
    auto cam = getGameplayCamera();
    cam->updateLocation({0, 2.8, 10});
    cam->updateLookupTarget({0, 1, -2});
    cameraMover = new CameraMover(cam);
}

void TestGame1::update() {
    cameraMover->update();
}

void TestGame1::render() {

    static Light* sun = nullptr;
    if (!sun) {
        sun = new Light();

        sun->location = {-3, 8,1 };
        sun->lookAtTarget = {0,0, 0};
        sun->shadowMapFBO = createShadowMapFramebufferObject({1024, 1024});
    }

    MeshDrawData dd;
    dd.mesh = getMeshByName("mech");
    dd.location = {0, 0, -8};
    dd.texture = getTextureByName("mech_albedo");
    dd.normalMap = getTextureByName("mech_normal");
    dd.uvScale = 1;
    dd.directionalLight = sun;
    dd.camera = getGameplayCamera();
    dd.shader = mechShader;
    drawMesh(dd);

    dd.location = {-3, 0, -4};
    drawMesh(dd);

    dd.location = {4, 0, -7.3};
    drawMesh(dd);


    dd.mesh = getMeshByName("ground_plane");
    dd.texture = getTextureByName("ground_albedo");
    dd.normalMap = getTextureByName("debug_normal");
    dd.scale = {5, 1, 5};
    dd.uvScale = 45;
    drawMesh(dd);
    renderFPS();
}

bool TestGame1::shouldStillRun() {
    return true;
}

bool TestGame1::shouldAutoImportAssets() {
    return true;
}

std::vector<std::string> TestGame1::getAssetFolder() {
    return {"../games/opengl/testgame1/assets"};
}
