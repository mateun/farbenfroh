//
// Created by mgrus on 03.02.2025.
//

#include "ServerViz.h"



DefaultGame* getGame() {
    return new ServerViz();
}

void ServerViz::init() {
    DefaultGame::init();
    cameraMover = new CameraMover(getGameplayCamera());
}

void ServerViz::update() {
    cameraMover->update();
}

void ServerViz::render() {

    bindCamera(getGameplayCamera());
    lightingOn();

    scale({1, 1,1});
    foregroundColor({0.6, 0.5, 0.6, .2});
    location(glm::vec3{0, 0, 0});
    gridLines(100);
    drawGrid();

    // Our ground plane
    uvScale(15);
    bindTexture(getTextureByName("groundplane"));
    location(glm::vec3{0, 0.01, 0});
    scale({100, 100, 1});
    rotation({-90, 0, 0});
    drawPlane();
    rotation({0, 0, 0});

    // // Ship
    uvScale(1);
    scale({1,1,1});
    flipUvs(true);
    bindMesh(getMeshByName("cube"));
    bindTexture(getTextureByName("cube_diffuse"));
    location(glm::vec3{0, 1, -2});
    drawMesh();
    location({0, 0, 0});

    foregroundColor({0.9, 0.2, 0.2, .5});
    renderFPS();

}

bool ServerViz::shouldStillRun() {
    return true;
}

bool ServerViz::shouldAutoImportAssets() {
    return true;
}

std::vector<std::string> ServerViz::getAssetFolder() {
    return { "../games/opengl/server_viz/assets", "../assets/sound"};
}
