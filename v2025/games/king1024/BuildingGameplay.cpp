//
// Created by mgrus on 01.11.2024.
//

#include "BuildingGameplay.h"
#include <glm/gtc/noise.hpp>
#include "../src/engine/math/data_structures.h"

king::BuildingGameplay::BuildingGameplay(king::KingGame *game) : KingGameState(game) {
    // Run tests
    auto bt = new BinaryTree<int>();
    bt->add(new BinaryTree(8));
    bt->add(new BinaryTree(4));
    bt->add(new BinaryTree(5));
    bt->add(new BinaryTree(123));

    bt->print();

    // end tests
}

void king::BuildingGameplay::resetCursorToCurrentView() {
    // Initialise the cursor position in the center of our view:
    // Override virtual mouse with analog sticks:
    mouse_x = scaled_width / 2;
    mouse_y = scaled_height/ 2;
    auto ray = createRayFromCameraAndMouse(kingGame->getGameplayCamera());
    rayIntersectsPlane(ray, {0, 1, 0}, {1, 0, 0}, &cursorLocation);
    cursorLocation.y = 0.85;

    // Temp debug: set location to world 0, 1, 0
    //cursorLocation = {0, 1, 0};

    // end debug
}

bool king::BuildingGameplay::costCheck(BuildingType buildingType) {
    int goldCost = 0;
    int woodCost = 0;
    int stoneCost = 0;
    if (buildingType == BuildingType::House) {
        goldCost = 100;
        woodCost = 50;
        stoneCost = 50;
    }
    // TODO other buildings costs
    if (kingGame->getGold() >= goldCost &&
        kingGame->getWood() >= woodCost &&
        kingGame->getStone() >= stoneCost) {
        return true;
    } else {
        return false;
    }




}

void king::BuildingGameplay::onUserPlacedBuilding() {

    {

        auto b = new Building();
        if (selBuildingIndex == 0) {
            b->type = BuildingType::House;
        } else if (selBuildingIndex == 1) {
            b->type = BuildingType::Barrack;
        }

        if (!costCheck(b->type)) {
            // TODO actually add error message
            // or do not enable the build menu in the first place.
            return;
        }

        b->mapLocation = {cursorLocation.x, -cursorLocation.z};
        buildings.push_back(b);
    }
}

void king::BuildingGameplay::update() {

    kingGame->getCameraMover()->update();

    if (controllerButtonPressed(ControllerButtons::Y_BUTTON,0 )) {
        resetCursorToCurrentView();
    }


    if (controllerButtonPressed(ControllerButtons::B_BUTTON, 0)) {
        bool eaten = false;
        if (mode == EGamePlayMode::BuildingPlacement) {
            mode = EGamePlayMode::Normal;
            eaten = true;

        }
        if (mode == EGamePlayMode::Normal && !eaten) {
            mode = EGamePlayMode::BuildingSelection;
            eaten = true;
        }

        if (mode == EGamePlayMode::BuildingSelection && !eaten) {
            mode = EGamePlayMode::Normal;
            eaten= true;
        }

    }

    if (controllerButtonPressed(ControllerButtons::A_BUTTON, 0)) {
        bool eaten = false;
        if (mode == EGamePlayMode::BuildingSelection) {
            eaten=true;
            mode = EGamePlayMode::BuildingPlacement;
            resetCursorToCurrentView();

        }

        if (mode == EGamePlayMode::BuildingPlacement && !eaten) {
            eaten=true;
            mode = EGamePlayMode::Normal;

           onUserPlacedBuilding();
        }

    }


    if (controllerButtonPressed(ControllerButtons::VIEW, 0)) {
        kingGame->switchState(EGameState::Exit, true, true);
    }

    if (mode == EGamePlayMode::BuildingSelection) {
        if (controllerButtonPressed(ControllerButtons::DPAD_LEFT, 0)) {
            if (selBuildingIndex > 0) {
                selBuildingIndex--;
            }
        }
        if (controllerButtonPressed(ControllerButtons::DPAD_RIGHT, 0)) {
            if (selBuildingIndex < 7) {
                selBuildingIndex++;
            }
        }
    }

    if (mode == EGamePlayMode::BuildingPlacement) {
        // Analog handling
        auto hdir = getControllerAxis(ControllerAxis::LSTICK_X, 0);
        auto vdir = getControllerAxis(ControllerAxis::LSTICK_Y, 0);

        if (hdir < -0.4) {
            hdir = -1;
        }
        else if (hdir > 0.4) {
            hdir = 1;
        } else {
            hdir = 0;
        }

        if (vdir < -0.4) {
            vdir = -1;
        }
        else if (vdir > 0.4) {
            vdir = 1;
        } else {
            vdir = 0;
        }

        static float tempX = 0;

        tempX += (float)abs(hdir) * ftSeconds * 250.0f;
        if (tempX > 1) {
            mouse_x += hdir;
            tempX = 0;
        }

        static float tempY = 0;
        tempY += (float)abs(vdir) * ftSeconds * 250.0f;
        if (tempY > 1) {
            mouse_y += vdir;
            tempY = 0;
        }



        auto ray = createRayFromCameraAndMouse(kingGame->getGameplayCamera());
        if (rayIntersectsPlane(ray, {0, 1, 0}, {1, 0, 0}, &cursorLocation)) {
            printf("intersects at %f/%f/%f\n", cursorLocation.x, cursorLocation.y, cursorLocation.z);
            cursorLocation.y = 0.85;
        };


// Digital handling
//        auto left = controllerButtonPressed(ControllerButtons::DPAD_LEFT, 0) ? -1 : 0;
//        auto right = controllerButtonPressed(ControllerButtons::DPAD_RIGHT, 0) ? 1 : 0;
//        auto up = controllerButtonPressed(ControllerButtons::DPAD_UP, 0) ? -1 : 0;
//        auto down = controllerButtonPressed(ControllerButtons::DPAD_DOWN, 0) ? 1 : 0;
//        auto hdir = left + right;
//        auto vdir = down + up;
//        cursorLocation.x += hdir;
//        cursorLocation.z += vdir;
//        cursorLocation.y = 0.90;

    }
}

void king::BuildingGameplay::render() {

    bindCamera(kingGame->getGameplayCamera());
    bindShadowMapCamera(kingGame->getShadowMapCamera());

    // Draw Grid for orientation
    {
        scale({1, 1, 1});
        foregroundColor({0.3, 0.6, 0.2, .1});
        location(glm::vec3{0, 0, 0});
        gridLines(100);

    }


    // Draw the actual terrain
    {
        deferredStart();
        shadowOn();
        bindTexture(kingGame->getTextureByName("block_diff"));
        bindMesh(kingGame->getMeshByName("grass-block"));
        lightingOn();
        flipUvs(true);
        std::vector<glm::vec2> offsets;

        // Terrain blocks
        {
            for (int x = 0; x < terrainSize.x; x++) {
                for (int y = 0; y < terrainSize.y; y++) {
                    auto terrainVal = terrainType[x + y * terrainSize.x];
                    if ( terrainVal == 2 || terrainVal == 3 || terrainVal == 4) {
                        offsets.push_back({x, -y});
                    }
                }
            }
            instanceOffsets(offsets);
            drawMeshInstanced(offsets.size());
        }

        // Trees
        {

            bindMesh(kingGame->getMeshByName("tree"));
            offsets.clear();
            for (int x = 0; x < terrainSize.x; x++) {
                for (int y = 0; y < terrainSize.y; y++) {
                    if (terrainType[x + y * terrainSize.x] == 3) {
                        offsets.push_back({x, -y});
                    }
                }
            }
            instanceOffsets(offsets);
            drawMeshInstanced(offsets.size());
        }

        // Rocks
        {
            offsets.clear();
            bindMesh(kingGame->getMeshByName("rocks"));
            //bindMesh(kingGame->getMeshByName("Saloon"));
            for (int x = 0; x < terrainSize.x; x++) {
                for (int y = 0; y < terrainSize.y; y++) {
                    if (terrainType[x + y * terrainSize.x] == 4) {
                        offsets.push_back({x, -y});
                    }
                }
            }
            instanceOffsets(offsets);
            //scale({0.1, 0.1, 0.1});
            drawMeshInstanced(offsets.size());

        }

        // Render buildings
        {

            bindTexture(kingGame->getTextureByName("tiny_buildings_blue_diffuse"));
            std::vector<Building*> houses;
            std::vector<Building*> barracks;

            for (auto b : buildings) {
                if (b->type== BuildingType::House) {
                    houses.push_back(b);
                } else if (b->type == BuildingType::Barrack) {
                    barracks.push_back(b);
                }
            }

            offsets.clear();
            bindMesh(kingGame->getMeshByName("med_house"));
            for (auto h : houses) {
                offsets.push_back({h->mapLocation.x, -h->mapLocation.y});
            }
            instanceOffsets(offsets);
            drawMeshInstanced(offsets.size());

            offsets.clear();
            bindMesh(kingGame->getMeshByName("med_barracks"));
            for (auto b : barracks) {
                offsets.push_back({b->mapLocation.x, -b->mapLocation.y});
            }
            instanceOffsets(offsets);
            drawMeshInstanced(offsets.size());


        }

        // Render units (moving)
        {

        }


        deferredEnd();

        flipUvs(false);
        shadowOff();
    }



    auto tileType = -1;
    // Cursor
    if (mode == EGamePlayMode::BuildingPlacement) {
        bindMesh(kingGame->getMeshByName("cursor"));
        bindTexture(nullptr);
        scale({1, 1, 1});
        cursorLocation.x = floor(cursorLocation.x);
        cursorLocation.z = floor(cursorLocation.z);

        if ((cursorLocation.x + cursorLocation.z * terrainSize.x) < ((terrainSize.x*terrainSize.y)-1) && cursorLocation.x >= 0 && cursorLocation.z <= 0) {
            tileType = terrainType[(int)cursorLocation.x + ((int) cursorLocation.z * -1 * terrainSize.x)];
            if ( tileType > 1) {
                foregroundColor({1, 1, 1, 1});
            } else {
                foregroundColor({1, 0, 0, 1});
            }
        } else {
            foregroundColor({1, 0, 0, 1});
        }

        location(cursorLocation);
        drawMesh();



    }

    // Ocean
    {
        bindTexture(nullptr);
        foregroundColor({0, 0, 1, 0.95});
        scale({800, 800, 1});
        location({0, 0.2, 0});
        rotation({-90, 0, 0});
        drawPlane();
        rotation({0, 0, 0});
    }

    // Terrain minimap
    {
        bindTexture(terrainTexture);
        lightingOff();
        bindCamera(kingGame->getUICamera());
        scale({(200*dpiScaleFactor), 200*dpiScaleFactor, 1});
        rotation({0, 0, 90});
        location({scaled_width - (300/dpiScaleFactor), (400/dpiScaleFactor), -2});
        flipUvs(true);
        drawPlane();
        rotation({0, 0, 0});
        flipUvs(false);
    }

    // UI

    if (mode == EGamePlayMode::BuildingSelection) {
        scale({scaled_width / (20/dpiScaleFactor), scaled_height / 4, 1});
        bindTexture(nullptr);
        foregroundColor({0, 0, 1, 1});
        bindTexture(preRenderFrambebuffer->texture);
        for (int i = 0; i < 8; i++) {
            location({scaled_width / 3 + (i * (scaled_width / (15/dpiScaleFactor))) , scaled_height / 2 - (100/dpiScaleFactor), -1.5});
            drawPlane();
            if (selBuildingIndex == i ){
                foregroundColor({1, 1, 1, 1});
                scale({scaled_width / 20 + (10/dpiScaleFactor), scaled_height / 4 + (10/dpiScaleFactor), 1});
                location({scaled_width / 3 + (i * (scaled_width / (15)/dpiScaleFactor)) , scaled_height / 2 - (100/dpiScaleFactor), -1.6});
                drawPlane();
                foregroundColor({0, 0, 1, 1});
            }
        }

    }

    if (mode == EGamePlayMode::BuildingPlacement) {


        glDefaultObjects->currentRenderState->textScale = {5, 5};
        foregroundColor({1, 1,1, 1});
        //kingGame->getButtonFont()->renderText("BUILDING MODE!", {scaled_width/2 + 256 * 4, scaled_height/2, -1});
        glDefaultObjects->currentRenderState->textScale = {1, 1};
        foregroundColor({1, 1,0, 1});
        std::string tileTypeDebugText = "tileType: " + std::to_string(tileType) + "[" + std::to_string(cursorLocation.x) + "/" +
                std::to_string(cursorLocation.z) + "] index: " + std::to_string(cursorLocation.x + cursorLocation.z * -1 * 100);
        kingGame->getButtonFont()->renderText(tileTypeDebugText, {scaled_width/2, scaled_height/2 - 300, -1});

    }
    kingGame->renderFPS();

}

void createTerrainWithPerlinNoise(uint8_t* terrainTiles, uint8_t* pixels, int width, int height) {
    int pixelIndex = 0;
    float noiseStepper = 0.03;
    float noiseX = 0;
    float noiseY = 0;


    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            noiseY += noiseStepper;
            float noiseValue = glm::perlin(glm::vec2{noiseX, noiseY});
            noiseValue = (noiseValue+1) * 0.5;

            // Island
            if (noiseValue < 0.55) {
                terrainTiles[x + y * width] = 2;
                pixels[pixelIndex] = 100;
                pixels[pixelIndex+1] = 250;
                pixels[pixelIndex+2] = 100;
                pixels[pixelIndex+3] = 255;

                // Trees
                if (noiseValue < 0.25) {
                    terrainTiles[x + y * width] = 3;
                    pixels[pixelIndex] = 200;
                    pixels[pixelIndex+1] = 250;
                    pixels[pixelIndex+2] = 200;
                    pixels[pixelIndex+3] = 255;
                }

                // Stone
                if (noiseValue > 0.36 && noiseValue < 0.361) {
                    terrainTiles[x + y * width] = 4;
                    pixels[pixelIndex] = 100;
                    pixels[pixelIndex+1] = 100;
                    pixels[pixelIndex+2] = 100;
                    pixels[pixelIndex+3] = 255;

                }
            }

            // Water
            if (noiseValue >= 0.55) {
                terrainTiles[x + y  * width] = 1;
                pixels[pixelIndex] = 100;
                pixels[pixelIndex+1] = 100;
                pixels[pixelIndex+2] = 255;
                pixels[pixelIndex+3] = 255;
            }

            pixelIndex += 4;
        }
        noiseX += noiseStepper;
        noiseY = 0;
    }
}

void createTerrainNonRandom(uint8_t* terrainTiles, int width, int height) {

}

void king::BuildingGameplay::init() {



    kingGame->getCameraMover()->setMovementSpeed(15);
    kingGame->getGameplayCamera()->updateLocation({5, 10, -10});

    terrainSize = {100, 100};
    terrainType = (uint8_t*) malloc(terrainSize.x* terrainSize.y);
    for (int i = 0; i < terrainSize.x*terrainSize.y; i++) {
        terrainType[i] = 1;
    }
    uint8_t* pixels = (uint8_t*) malloc(terrainSize.x * terrainSize.y * 4);
    createTerrainWithPerlinNoise(terrainType, pixels, terrainSize.x, terrainSize.y);

    auto bitmap = new Bitmap(terrainSize.x, terrainSize.y, pixels);
    terrainTexture = createTextureFromBitmap(bitmap);
    free(pixels);
    delete(bitmap);
    glDefaultObjects->currentRenderState->clearColor = {0.5, 0.5, 1, 1};

    // prerender the buildings
    {

        auto cam = Camera();
        cam.type = CameraType::Perspective;
        cam.updateLocation({0, 0, 5});
        cam.updateLookupTarget({0, 0, -5});
        bindCamera(&cam);
        preRenderFrambebuffer  = createFrameBuffer(1024, 1024);
        housePreRenderedTexture = preRenderFrambebuffer->texture;
        lightingOn();
        activateFrameBuffer(preRenderFrambebuffer);
        glViewport(0, 0, 1024,
                   1024);

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //foregroundColor({0, 0, 1, 1});
        bindTexture(kingGame->getTextureByName("tiny_buildings_blue_diffuse"));

        // Make sure to display the texture correctly
        //flipUvs(true);

        bindMesh(kingGame->getMeshByName("med_house"));
        location({0, 0, 0});
        //scale({2, 2, 1});
        drawMesh();
        activateFrameBuffer(nullptr);
        glViewport(0, 0, scaled_width, scaled_height);
    }

}
