//
// Created by mgrus on 09.03.2025.
//

#include "LevelEditor.h"
#include <engine/game/default_game.h>




LevelEditor::LevelEditor(DefaultGame *game, const std::string &name) : GameLevel(game, name) {
}

void LevelEditor::render() {

    // MeshDrawData mdd;
    // mdd.mesh = quadMesh;
    // mdd.shader = game->getDefaultStaticMeshShader();
    // mdd.location = {0, 0, -1};
    // mdd.scale = {300, scaled_height - 20, 1};
    // mdd.color = {1, 1, 0.5, 0.5};
    // mdd.camera = game->getUICamera();
    // mdd.subroutineFragBind = "calculateSingleColor";
    // drawMesh(mdd);
    //
    // mdd.location = {302, 0, -1};
    // drawMesh(mdd);

    baseWidget->setPosition(100, 100);
    baseWidget->setScale(400, 800);
    baseWidget->render();
    game->renderFPS();
}

void LevelEditor::update() {
    printf("in editor");
}

void LevelEditor::init() {
    quadMesh = createQuadMesh(PlanePivot::bottomleft);
    baseWidget = new Widget();

}


