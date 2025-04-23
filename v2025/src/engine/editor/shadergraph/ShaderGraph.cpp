//
// Created by mgrus on 12.02.2025.
//

#include "ShaderGraph.h"
#include <engine/io/io.h>
#include <stdexcept>
#include <engine/graphics/Camera.h>
#include <glm/gtc/type_ptr.inl>
#include <engine/graphics/ErrorHandling.h>
#include <engine/graphics/Renderer.h>
#include <engine/graphics/StatefulRenderer.h>

#ifdef ACTIVE_SHADER_GRAPH
DefaultGame* getGame() {
    return new ShaderGraph();
}
#endif

void ShaderGraph::init() {
    DefaultGame::init();
    quadVAO = StatefulRenderer::createQuadVAO();
    nodeShader = new Shader();
    auto nodeVert = readFile("../src/engine/editor/shadergraph/shaders/node.vert");
    auto nodeFrag= readFile("../src/engine/editor/shadergraph/shaders/node.frag");
    if (!StatefulRenderer::createShader(nodeVert, nodeFrag, nodeShader).ok) {
        throw std::runtime_error("Error creating node shader program");
    }
}

void ShaderGraph::update() {

}

void ShaderGraph::setTransformMatrices(glm::vec3 location, glm::vec3 scale, glm::vec3 rotation) {
    using namespace glm;

    // Object to world transformation
    mat4 mattrans = translate(mat4(1), location);
    mat4 matscale = glm::scale(mat4(1),scale);
    mat4 matworld = glm::mat4(1);

    mat4 matrotX = glm::rotate(mat4(1), glm::radians(rotation.x), {1, 0, 0} );
    mat4 matrotY = glm::rotate(mat4(1), glm::radians(rotation.y), {0, 1, 0} );
    mat4 matrotZ = glm::rotate(mat4(1), glm::radians(rotation.z), {0, 0, 1} );
    matworld =  mattrans * matrotX * matrotY * matrotZ * matscale ;

    nodeShader->setMat4Value(matworld, "mat_model");
    nodeShader->setMat4Value(getUICamera()->getViewMatrix(), "mat_view");
    nodeShader->setMat4Value(getUICamera()->getProjectionMatrix(), "mat_projection");

}
void ShaderGraph::renderNode() {
    renderTile({0,0}, {16, 16} ,{200, 200 + 8 + (128)}, {16, 16});
    renderTile({1, 0}, {16, 16}, {200 + (128/2), 200}, {128, 16});
    renderTile({2, 0}, {16, 16}, {(200-8) + (128) + 16 , 200}, {16, 16});
    renderTile({0, 1}, {16, 16}, {200  , 200 + (128/2) }, {16, 128});
    renderTile({2, 1}, {16, 16}, {(200-8) + (128) + 16  , 200 + (128/2) }, {16, 128});
    renderTile({1, 1}, {16, 16}, {(200-8) + (128/2) + 16  , 200 + (128/2) }, {128, 128});
}


void ShaderGraph::renderTile(glm::vec2 tileCoord, glm::vec2 tileSize, glm::vec2 screenCoord, glm::vec2 scale) {
    glBindVertexArray(quadVAO);

    nodeShader->bind();
    GL_ERROR_EXIT(45);
    glActiveTexture(GL_TEXTURE0);
    GL_ERROR_EXIT(46);
    glBindTexture(GL_TEXTURE_2D, getTextureByName("node-9-slice")->handle());
    GL_ERROR_EXIT(47);

    // Manipulate the uvs for tiling.

    float bmSize = 48;
    glm::vec2 tileOffsetInPixels = {0, 0};
    glm::vec2 tileOffsetInUVs = {0,0};

    //glUniform1i(20, 0);
    float width_uv = (float)  tileSize.x / bmSize ;
    float height_uv = (float) tileSize.y / bmSize;

    tileOffsetInUVs.x = (float) tileOffsetInPixels.x / bmSize;
    tileOffsetInUVs.y = (float) tileOffsetInPixels.y / bmSize;
    float uvs[] = {
            tileOffsetInUVs.x + (tileCoord.x * width_uv) , ( 1 - tileOffsetInUVs.y - (tileCoord.y * height_uv)),
            tileOffsetInUVs.x + (tileCoord.x * width_uv), (1- tileOffsetInUVs.y - (tileCoord.y * height_uv) - height_uv),
            tileOffsetInUVs.x + (tileCoord.x * width_uv) + width_uv,  (1- tileOffsetInUVs.y -  (tileCoord.y * height_uv)- height_uv),
            tileOffsetInUVs.x + (tileCoord.x * width_uv) + width_uv, (1 - tileOffsetInUVs.y - (tileCoord.y * height_uv)),
    };
    //glBindBuffer(GL_ARRAY_BUFFER, glDefaultObjects->quadUVBuffer);
    glBufferData(GL_ARRAY_BUFFER, 12 * 4, uvs, GL_STATIC_DRAW);

    setTransformMatrices({screenCoord.x, screenCoord.y, -1}, {scale.x, scale.y, 1}, {0, 0, 0});

    // glUniform1f(21, glDefaultObjects->currentRenderState->uvScale);
    //
    // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    // GL_ERROR_EXIT(344)
    //
    // if (glDefaultObjects->currentRenderState->tilingOn) {
    //     glUniform1i(20, 0);
    //     float uvs[] = {
    //             0, 1,
    //             0, 0,
    //             1, 0,
    //             1, 1,
    //
    //     };
    //     glBindBuffer(GL_ARRAY_BUFFER, glDefaultObjects->quadUVBuffer);
    //     glBufferData(GL_ARRAY_BUFFER, 12 * 4, uvs, GL_STATIC_DRAW);
    // }
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
}

void ShaderGraph::render() {
    renderNode();
    renderFPS();

}

bool ShaderGraph::shouldStillRun() {
    return true;
}

bool ShaderGraph::shouldAutoImportAssets() {
    return true;
}

std::vector<std::string> ShaderGraph::getAssetFolder() {
    return {"../src/engine/editor/assets"};
}
