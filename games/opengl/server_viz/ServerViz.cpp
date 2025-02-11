//
// Created by mgrus on 03.02.2025.
//

#include "ServerViz.h"
#include "../src/engine/Light.h"


DefaultGame* getGame() {
    return new ServerViz();
}

void ServerViz::init() {
    DefaultGame::init();
    cameraMover = new CameraMover(getGameplayCamera());
    _fullscreenFrameBuffer = createFrameBuffer( scaled_width, scaled_height);
    _postProcessedFrameBuffer = createFrameBuffer( scaled_width, scaled_height);

    auto vertSource = readFile("../games/opengl/server_viz/assets/shaders/post_process.vert");
    auto fragSource = readFile("../games/opengl/server_viz/assets/shaders/post_process.frag");
    _postProcessShader = new Shader();
    createShader(vertSource, fragSource, _postProcessShader);

    _cameraIn3DWorld = new Camera();
    _cameraIn3DWorld = new Camera();
    _cameraIn3DWorld->location = {0, 1.5, 1};
    _cameraIn3DWorld->lookAtTarget = {0, 1.5, -5};
    _cameraIn3DWorld->type = CameraType::Perspective;

    _skyboxTexture = createCubeMapTextureFromDirectory("../games/opengl/server_viz/assets/skybox", ColorFormat::RGBA, "png");
    _skyboxFront = createTextureFromFile("../games/opengl/server_viz/assets/skybox/front.jpg");

    // Setup the actual scene
    scene = new Scene();
    SceneNode* mainCameraNode = new SceneNode();
    mainCameraNode->type = SceneNodeType::Camera;
    mainCameraNode->camera = getGameplayCamera();
    scene->addNode(mainCameraNode);

    Light* sunlight = new Light();
    sunlight->type = LightType::Directional;
    sunlight->location = {-8, 10, 3};
    sunlight->lookAtTarget = {0, 0, 0};
    sunlight->castsShadow = true;
    sunlight->shadowMapFBO = createShadowMapFramebufferObject({1024, 1024});

    SceneNode* sunNode = new SceneNode();
    sunNode->type = SceneNodeType::Light;
    sunNode->light = sunlight;
    scene->addNode(sunNode);

    auto lampNode = new SceneNode();
    lampNode->type = SceneNodeType::Light;
    lampNode->light = new Light();
    lampNode->light->type = LightType::Point;
    lampNode->light->location = {-2, 0.8, 3};
    lampNode->light->lookAtTarget = {0, 0, 0};
    lampNode->light->castsShadow = false;
    lampNode->light->shadowMapFBO = createShadowMapFramebufferObject({1024, 1024});
    scene->addNode(lampNode);

    SceneNode* mnGroundPlane = new SceneNode();
    mnGroundPlane->type = SceneNodeType::Mesh;
    mnGroundPlane->location = {0, 0, 0};
    mnGroundPlane->mesh = getMeshByName("cube");
    mnGroundPlane->texture = getTextureByName("street_albedo");
    mnGroundPlane->normalMap = getTextureByName("street_normal");
    mnGroundPlane->uvScale = 75;
    mnGroundPlane->scale = {55, .1, 55};
    scene->addNode(mnGroundPlane);

    auto mnCube = new SceneNode();
    mnCube->type = SceneNodeType::Mesh;
    mnCube->location = {0, 1, 0};
    mnCube->mesh = getMeshByName("cube");
    mnCube->texture = getTextureByName("wood_albedo");
    mnCube->normalMap = getTextureByName("wood_normal");
    mnCube->uvScale = 1;
    mnCube->scale = {1, 1, 1};
    scene->addNode(mnCube);

    auto mnCube2 = new SceneNode();
    mnCube2->type = SceneNodeType::Mesh;
    mnCube2->location = {-0.5, 3, -0.2};
    mnCube2->rotation = {0, 10, 0};
    mnCube2->mesh = getMeshByName("cube");
    mnCube2->texture = getTextureByName("wood_albedo");
    mnCube2->normalMap = getTextureByName("wood_normal");
    mnCube2->uvScale = 1;
    mnCube2->scale = {1, 1, 1};
    //scene->addNode(mnCube2);



}

void ServerViz::updateLoadingScreen() {

}

// Render the title of the game
// The main menu "buttons"
// All on top of small 3D scene inside the Arcade shop.
// Wachting the cabinet, being dimly lit, some particles moving in the air.
// TODO: create particles and light them, move them like dust.
void ServerViz::renderMainMenu() {
    static float mainMenuTime = 0;
    mainMenuTime += ftSeconds;

    bindCamera(getGameplayCamera());
    lightingOn();

    // rotation({0,0, 0});
    // scale({1,1,1});
    // static auto gd = createGrid(25);
    // drawGrid(gd);

    bindTexture(getTextureByName("wood_albedo"));
    bindNormalMap(getTextureByName("wood_normal"));
    foregroundColor({0.8, 0.8, 0.8, 1});
    auto mesh = getMeshByName("cabinet3");
    bindMesh(mesh);
    location({2, 1.5, -0});
    rotation({0, 15, 0});
    scale({1, 1, 1});
    tint({1,1, 1,1});
    uvScale(2);
    drawMesh();
    uvScale(1.0);

    bindCamera(getUICamera());
    lightingOff();
    flipUvs(true);
    bindTexture(getTextureByName("title_space_patrol"));
    location({window_width/2, window_height - 156, -2.1});
    scale({1280, 256, 1});
    rotation({0, 0, 0});
    tint({1, 1, 1, 1});
    drawPlane();

    renderFPS();
}

// This just renders a loading screen (duh..) for a few seconds
void ServerViz::renderLoadingScreen() {
    static float loadingScreenTime = 0;
    loadingScreenTime += ftSeconds;
    static float appearAlpha = 0;

    if (keyPressed(VK_SPACE)) {
        _state = State::MainMenu;
        return;
    }

    if ( loadingScreenTime < 2) {
        appearAlpha += 0.4 * ftSeconds;
        if (appearAlpha > 1) {
            appearAlpha = 1;
        }
    }

    if (loadingScreenTime >= 2.5) {
        appearAlpha -= 0.5 * ftSeconds;
    }

    if (appearAlpha < 0) {
       _state = State::MainMenu;
    }

    bindCamera(getUICamera());
    lightingOff();
    flipUvs(true);
    bindTexture(getTextureByName("loading_screen"));
    location({scaled_width/2, scaled_height/2, -0.1});
    scale({scaled_width, scaled_height, 1});

    tint({1, 1, 1, appearAlpha});
    drawPlane();
}

void ServerViz::renderArcade() {
    bindCamera(getGameplayCamera());
    lightingOn();

    scene->render();
    return;

    // Prepare framebuffer for fullscreen post-processing
    {
        activateFrameBuffer(_fullscreenFrameBuffer);
        glViewport(0, 0, _fullscreenFrameBuffer->texture->bitmap->width,
                    _fullscreenFrameBuffer->texture->bitmap->height);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }


    bindSkyboxTexture(_skyboxTexture);
    drawSkybox();
    unbindSkyboxTexture();


    // Our ground plane
    bindMesh(getMeshByName("cube"));
    bindTexture(getTextureByName("wood_albedo"));
    bindNormalMap(getTextureByName("wood_normal"));
    uvScale(55);
    location(glm::vec3{0, 0, 0});
    scale({55, .1, 55});
    drawMesh();

    // Some street tiles
    bindMesh(getMeshByName("street"));
    bindTexture(getTextureByName("street_albedo"));
    bindNormalMap(getTextureByName("street_normal"));
    uvScale(1);
    scale({1, 1, 1});
    for (int i = 0; i < 8; i++) {
        location(glm::vec3{-3 + (i* 2.1), 0, -4});
        drawMesh();
    }


    // Ship
    uvScale(1);
    scale({1,1,1});
    flipUvs(true);
    bindMesh(getMeshByName("cube"));
    bindTexture(getTextureByName("wood_albedo"));
    bindNormalMap(getTextureByName("wood_normal"));
    location(glm::vec3{0, 1, -2});
    drawMesh();
    location({0, 0, 0});



    // Deactivate the full screen framebuffer and render the texture as
    // a quad into the postprocessed framebuffer.
    // This will be the basis of what we display in our 3d world.
    {
        activateFrameBuffer(_postProcessedFrameBuffer);
        glViewport(0, 0, _postProcessedFrameBuffer->texture->bitmap->width,
                       _postProcessedFrameBuffer->texture->bitmap->height);
        glClearColor(0.6, 0.1, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    lightingOff();
    bindCamera(getUICamera());
    bindTexture(_fullscreenFrameBuffer->texture);
    location({window_width/2, window_height/2, -0.1});
    static float rot = 0;
    if (fade) {
        //rot += 1000 * ftSeconds;
    } else {
        rot = 0;
    }
    rotation({0, 0, 0});
    static float scaleFactor = 1.0f;
    if (fade) {
        //scaleFactor += 30 * ftSeconds;
    } else {
        scaleFactor = 1;
    }
    static float tinto = 1.0f;
    if (fade) {
        tinto -= 1 * ftSeconds;
        tint({1, 1, 1, tinto});
    }
    scale({window_width/scaleFactor, window_height/scaleFactor, 1});
    forceShader(_postProcessShader);
    setUniformFloat(10, ftSeconds * 4000, _postProcessShader);
    // scaled_width = 1024;
    // scaled_height = 768;
    drawPlane();
    rotation({0, 0, 0});
    scale({1, 1, 1});
    tint({1, 1, 1, 1});
    forceShader(nullptr);



    // Now back to the main backbuffer.
    // We take the last framebuffer, which has the final image of the
    // inner arcade game. We use this as a texture to project onto our cabinet screen.
    activateFrameBuffer(nullptr);
    glViewport(0, 0, scaled_width, scaled_height);

    // We draw our main 3D world through a separate camera,
    // which is not influenced by a mover.
    // Of course, if we go into world-mode (instead of arcade-mode),
    // this must change so the player can actually control inside the 3D world.
    // (Then we can ignore the rendering of the arcade world).
    bindCamera(getUICamera());

    // The screen is a bent plane mesh, which gets the current main image
    // as its texture.
    // It is not lit, everything lighing is already done in the rendering and post-processing.
    // Here we present the image as is on the "screen".
    bindMesh(getMeshByName("screen_plane"));
    scale({scaled_width, scaled_height, 1});
    //bindNormalMap(getTextureByName("wood_normal"));
    bindTexture(_postProcessedFrameBuffer->texture);
    flipUvs(false);
    location({scaled_width/2, scaled_height/2, -0.5});
    drawPlane();

    // TODO enable display of debug grid
    // currently this is not visible.
    // Even depthtest on/off did not help
    static auto gd = createGrid(75);
    location({0, 0.05, 0});
    scale({1,1,1});
    drawGrid(gd);


    foregroundColor({0.9, 0.2, 0.2, .5});
    renderFPS();
}

void ServerViz::updateMainMenu() {

    if (keyPressed(VK_SPACE)) {
        _state = State::Arcade;
        stopSound(getSoundByName("TripArcade"));
    }

    static float bgMusicOn = false;
    if (!bgMusicOn) {
        bgMusicOn = true;
        playSound(getSoundByName("TripArcade"), true);
    }

}

void ServerViz::updateArcade() {
    cameraMover->update();
    if (keyPressed('F')) {
        fade = !fade;
    }
}

void ServerViz::updateOutsideWorld() {

}

void ServerViz::updateSettings() {

}

void ServerViz::update() {

    if (_state == State::LoadingScreen) {
        updateLoadingScreen();
    }
    else if (_state == State::MainMenu) {
        updateMainMenu();
    }
    else if (_state == State::Arcade) {
        updateArcade();
    }



}

void ServerViz::render() {

    if (_state == State::LoadingScreen) {
        renderLoadingScreen();
    }
    else if (_state == State::MainMenu) {
        renderMainMenu();
    } else if (_state == State::Arcade) {
        renderArcade();
    }



}

bool ServerViz::shouldStillRun() {
    return true;
}

bool ServerViz::shouldAutoImportAssets() {
    return true;
}

std::vector<std::string> ServerViz::getAssetFolder() {
    return { "../games/opengl/server_viz/assets", "../games/opengl/server_viz/assets/sounds"};
}
