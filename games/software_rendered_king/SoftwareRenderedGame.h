//
// Created by mgrus on 06.12.2024.
//

#ifndef SOFTWARERENDEREDGAME_H
#define SOFTWARERENDEREDGAME_H
#include <vector>
#include <string>
#include "../src/engine/game/default_game.h"

class SoftwareRenderedGame : public DefaultGame {

    void update() override;
    void init() override;


    glm::vec3 transformObjectToScreenSpace(std::vector<glm::vec3>::value_type vec, glm::mat4 mat);

    void render() override;
    bool shouldStillRun() override;
    std::vector<std::string> getAssetFolder() override;
    bool shouldAutoImportAssets() override;
    void clearBackBuffer();
    void drawPixel(int x, int y, glm::vec4 color);
    void drawLine(glm::vec2 from, glm::vec2 to, glm::vec4 color);
    void drawTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec4 col);

    Texture * backbufferTexture = nullptr;
    glm::mat4 currentProjectionMatrix;
    glm::mat4 currentViewMatrix;
};

DefaultGame* getGame() {
  return new SoftwareRenderedGame();
}

#endif //SOFTWARERENDEREDGAME_H
