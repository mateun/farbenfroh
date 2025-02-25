//
// Created by mgrus on 25.02.2025.
//

#include "GamePlayLevel.h"
#include "../../../src/engine/game/default_app.h"

GamePlayLevel::GamePlayLevel(DefaultApp *game) : GameLevel(game) {
}

void GamePlayLevel::render() {
    scene->render();

}

void GamePlayLevel::update() {
    animationController->update();
    playerNode->updateBoneMatrices(animationController->getBoneMatrices());
}

void GamePlayLevel::init() {
    auto mechShader = new Shader();
    mechShader->initFromFiles("../games/opengl/testgame1/assets/shaders/mech.vert", "../games/opengl/testgame1/assets/shaders/mech.frag");
    auto skinnedShader = new Shader();
    skinnedShader->initFromFiles("../games/opengl/testgame1/assets/shaders/skinned.vert", "../games/opengl/testgame1/assets/shaders/mech.frag");

    scene = new Scene();


    auto cam = game->getGameplayCamera();
    cam->updateLocation({0, 10, 10});
    cam->updateLookupTarget({0, 0, -2});
    cam->setInitialForward(cam->getForward());
    //cameraMover = new CameraMover(cam);
    auto mainCamNode = new SceneNode();
    mainCamNode->initAsCameraNode(cam);
    scene->addNode(mainCamNode);


    auto mechNode = new SceneNode("mech");
    SceneMeshData smd;
    smd.mesh =  game->getMeshByName("mech");
    smd.texture = game->getTextureByName("mech_albedo");
    smd.normalMap = game->getTextureByName("mech_normal");
    smd.shader = mechShader;
    mechNode->initAsMeshNode(&smd);
    mechNode->setLocation(glm::vec3(0, 0, -5));
    scene->addNode(mechNode);

    playerNode = new SceneNode("player");
    //playerNode->disable();
    smd.mesh = game->getMeshByName("human4_oriented");
    smd.texture = game->getTextureByName("debug_texture");
    smd.normalMap = game->getTextureByName("mech_normal");
    smd.shader = skinnedShader;
    smd.skinnedMesh = true;
    playerNode->setLocation({-2, 0, 2});
    playerNode->initAsMeshNode(&smd);
    scene->addNode(playerNode);

    auto groundNode = new SceneNode("ground");
    //groundNode->disable();
    groundNode->setLocation (glm::vec3(0, 0, 0));
    groundNode->setScale(glm::vec3(20, 0.5, 20));
    smd.mesh = game->getMeshByName("ground_plane");
    smd.texture = game->getTextureByName("ground_albedo");
    smd.normalMap = game->getTextureByName("debug_normal");
    smd.uvScale = 205;
    smd.shader = mechShader;
    smd.skinnedMesh = false;
    groundNode->initAsMeshNode(&smd);
    scene->addNode(groundNode);

    // TODO also put lights back into more generic node mode.
    auto sun = new Light();
    sun->type = LightType::Directional;
    sun->color = glm::vec3(1, 1, 1);
    sun->location = {3, 3,8 };
    sun->lookAtTarget = {0,0, 0};
    sun->shadowMapFBO = createShadowMapFramebufferObject({1024, 1024});
    auto sunNode = new SceneNode();
    sunNode->initAsLightNode(sun);
    scene->addNode(sunNode);

    idlePlayer = new AnimationPlayer(game->getMeshByName("human4_oriented")->findAnimation("idle"), game->getMeshByName("human4_oriented"));
    walkPlayer = new AnimationPlayer(game->getMeshByName("human4_oriented")->findAnimation("walk"), game->getMeshByName("human4_oriented"));
    idlePlayer->play(true);

    animationController = new AnimationController(game->getMeshByName("human4_oriented"));
    auto idleState = new AnimationState(game->getMeshByName("human4_oriented")->findAnimation("idle"), "idle");
    auto walkState = new AnimationState(game->getMeshByName("human4_oriented")->findAnimation("walk"), "walk");
    animationController->addAnimationState(idleState);
    animationController->addAnimationState(walkState);
    auto idleWalkTrans = new AnimationTransition(animationController, idleState, walkState);
    auto idleWalkConstraint = new TransitionConstraint();
    idleWalkConstraint->propertyKey = "movementSpeed";
    idleWalkConstraint->property.floatValue = 0.5;
    idleWalkConstraint->property.propertyType = PropertyType::FLOAT;
    idleWalkConstraint->constraintOperator = ConstraintOperator::GREATEREQUAL;
    idleWalkTrans->addConstraint(idleWalkConstraint);

    animationController->update();
    playerNode->updateBoneMatrices(animationController->getBoneMatrices());

}
