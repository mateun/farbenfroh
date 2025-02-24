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
    skinnedShader = new Shader();
    skinnedShader->initFromFiles("../games/opengl/testgame1/assets/shaders/skinned.vert", "../games/opengl/testgame1/assets/shaders/mech.frag");

    scene = new Scene();


    auto cam = getGameplayCamera();
    cam->updateLocation({0, 1.8, 10});
    cam->updateLookupTarget({0, 1, -2});
    cam->setInitialForward(camFwd(cam));
    cameraMover = new CameraMover(cam);
    auto mainCamNode = new SceneNode();
    mainCamNode->initAsCameraNode(cam);
    scene->addNode(mainCamNode);


    auto mechNode = new SceneNode();
    SceneMeshData smd;
    smd.mesh =  getMeshByName("mech");
    smd.texture = getTextureByName("mech_albedo");
    smd.normalMap = getTextureByName("mech_normal");
    smd.shader = mechShader;
    mechNode->initAsMeshNode(&smd);
    mechNode->setLocation(glm::vec3(0, 0, -5));
    scene->addNode(mechNode);

    playerNode = new SceneNode();
    smd.mesh = getMeshByName("human4_oriented");
    smd.texture = getTextureByName("debug_texture");
    smd.normalMap = getTextureByName("mech_normal");
    smd.shader = skinnedShader;
    smd.skinnedMesh = true;

    playerNode->setLocation({-2, 0, 2});
    playerNode->initAsMeshNode(&smd);
    scene->addNode(playerNode);

    auto groundNode = new SceneNode();
    groundNode->setLocation (glm::vec3(0, 0, 0));
    groundNode->setScale(glm::vec3(20, 0.5, 20));
    smd.mesh = getMeshByName("ground_plane");
    smd.texture = getTextureByName("ground_albedo");
    smd.normalMap = getTextureByName("debug_normal");
    smd.uvScale = 205;
    smd.shader = mechShader;
    smd.skinnedMesh = false;
    groundNode->initAsMeshNode(&smd);
    scene->addNode(groundNode);

    // TODO also put lights back into more generic node mode.
    sun = new Light();
    sun->type = LightType::Directional;
    sun->location = {3, 3,8 };
    sun->lookAtTarget = {0,0, 0};
    sun->shadowMapFBO = createShadowMapFramebufferObject({1024, 1024});
    auto sunNode = new SceneNode();
    sunNode->initAsLightNode(sun);
    scene->addNode(sunNode);

    auto sun2 = new Light();
    sun2->type = LightType::Directional;
    sun2->location = {-3, 2,6 };
    sun2->lookAtTarget = {0,0, 0};
    sun2->shadowMapFBO = createShadowMapFramebufferObject({1024, 1024});
    auto sunNode2 = new SceneNode();
    sunNode2->initAsLightNode(sun2);
    scene->addNode(sunNode2);

    cinematic = new Cinematic(scene, 10);
    auto camTrack = cinematic->addTrack(mainCamNode, "mainCamTrack");
    camTrack->addKeyFrame(ChannelType::Location, 0, {0, 1.8, 10});
    camTrack->addKeyFrame(ChannelType::Location, 5, {-8, 0.8, -2});
    camTrack->addKeyFrame(ChannelType::Location, 10, {-8, 0.8, -2});
    // TODO fix rotation application
    camTrack->addKeyFrame(ChannelType::Rotation, 0, {0, 0, 0});
    camTrack->addKeyFrame(ChannelType::Rotation, 7, {0, -1.83f, 0});
    camTrack->addKeyFrame(ChannelType::Rotation, 8, {0, -1.83f, 0});
    camTrack->addKeyFrame(ChannelType::Rotation, 10, {0.2, -1.83f, 0});

    characterController = new CharacterController(playerNode);
    updateSwitcher = new UpdateSwitcher({characterController, cameraMover}, VK_F10);

    idlePlayer = new AnimationPlayer(getMeshByName("human4_oriented")->findAnimation("idle"), getMeshByName("human4_oriented"));
    walkPlayer = new AnimationPlayer(getMeshByName("human4_oriented")->findAnimation("walk"), getMeshByName("human4_oriented"));
    idlePlayer->play(true);

    animationController = new AnimationController(getMeshByName("human4_oriented"));
    auto idleState = new AnimationState(getMeshByName("human4_oriented")->findAnimation("idle"), "idle");
    auto walkState = new AnimationState(getMeshByName("human4_oriented")->findAnimation("walk"), "walk");
    animationController->addAnimationState(idleState);
    animationController->addAnimationState(walkState);
    auto idleWalkTrans = new AnimationTransition(animationController, idleState, walkState);
    auto idleWalkConstraint = new TransitionConstraint();
    idleWalkConstraint->propertyKey = "movementSpeed";
    idleWalkConstraint->property.floatValue = 0.5;
    idleWalkConstraint->property.propertyType = PropertyType::FLOAT;
    idleWalkConstraint->constraintOperator = ConstraintOperator::GREATEREQUAL;
    idleWalkTrans->addConstraint(idleWalkConstraint);

}

void TestGame1::update() {

    // Check for cinematic activation:
    {
        if (keyPressed('C')) {
            if (!cinematic->isActive()) {
                cinematic->play();
            }
        }
        if (cinematic->isActive()) {
            cinematic->update();
            return;
        }
    }

    updateSwitcher->update();

    // Temp. implement animation switching here
    //idlePlayer->update();
    //playerNode->boneMatrices = idlePlayer->getCurrentBoneMatrices();
    animationController->update();
    playerNode->updateBoneMatrices(animationController->getBoneMatrices());

    // A little test setup:
    // We wait for 2 seconds, then we set the animation controller property
    // to 1.0, expecting the transition from idle to walk to trigger.
    // (which is set to trigger when the movmentSpeed is >= 0.5)
    static float timePassed = 0;
    timePassed += ftSeconds;
    if (timePassed > 2.0) {
        AnimationProperty prop;
        prop.propertyType = PropertyType::FLOAT;
        prop.floatValue = 1.0f;
        animationController->setProperty("movementSpeed", prop);
    }
    // end temp


}

void TestGame1::render() {
    if (cinematic->isActive()) {
        cinematic->render();
        return;
    }

    scene->render();

    // MeshDrawData dd;
    // dd.mesh = getMeshByName("human4_oriented");
    // dd.location = {-1, 1, -4};
    // dd.texture = getTextureByName("mech_albedo");
    // dd.normalMap = getTextureByName("mech_normal");
    // dd.uvScale = 1;
    // dd.directionalLight = sun;
    // dd.camera = getGameplayCamera();
    // dd.shader = skinnedShader;
    // dd.skinnedDraw = true;
    // dd.boneMatrices = idlePlayer->getCurrentBoneMatrices();
    // drawMesh(dd);
    //
    // dd.location = {-3, 0, -4};
    // dd.rotationEulers = {0, 180, 0};
    // drawMesh(dd);
    //
    // dd.location = {4, 0, -7.3};
    // drawMesh(dd);
    //
    //
    // dd.skinnedDraw = false;
    // dd.mesh = getMeshByName("ground_plane");
    // dd.texture = getTextureByName("ground_albedo");
    // dd.normalMap = getTextureByName("debug_normal");
    // dd.scale = {5, 1, 5};
    // dd.uvScale = 45;
    // drawMesh(dd);
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
