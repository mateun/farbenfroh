//
// Created by mgrus on 12.02.2025.
//

#ifndef SHADERGRAPH_H
#define SHADERGRAPH_H

#include "../../game/default_app.h"


class ShaderGraph : public DefaultApp {

    void init() override;
    void update() override;
    void render() override;
    bool shouldStillRun() override;
    bool shouldAutoImportAssets() override;
    std::vector<std::string> getAssetFolder() override;

private:
    void setTransformMatrices(glm::vec3 location, glm::vec3 scale, glm::vec3 rotation);
    void renderNode();
    void renderTile(glm::vec2 tileCoord, glm::vec2 tileSize, glm::vec2 screenCoord, glm::vec2 scale = {1, 1});

    GLuint  quadVAO;
    Shader * nodeShader = nullptr;
};



#endif //SHADERGRAPH_H
