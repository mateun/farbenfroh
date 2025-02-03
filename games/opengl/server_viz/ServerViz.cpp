//
// Created by mgrus on 03.02.2025.
//

#include "ServerViz.h"



DefaultGame* getGame() {
    return new ServerViz();
}

void ServerViz::update() {
  

}

void ServerViz::render() {
    bindCamera(getGameplayCamera());
    lightingOn();

    scale({1, 1,1});
    foregroundColor({0.3, 0.6, 0.2, .1});
    location(glm::vec3{0, 0, 0});
    gridLines(100);
    drawGrid();

    renderFPS();

}

bool ServerViz::shouldStillRun() {
    return true;
}

bool ServerViz::shouldAutoImportAssets() {
    return true;
}

std::vector<std::string> ServerViz::getAssetFolder() {
    return { "../games/king1024/assets", "../assets/sound"};
}
