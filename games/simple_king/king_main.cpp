#include <stdio.h>
#include <time.h>
#include "graphics.h"
#include "engine/io/io.h"
#include <iostream>
#include <cctype>
#include <random>
#include <thread>
#include <map>
#include "engine/io/base64.h"
#include "engine/sound/sound.h"
#include <sstream>
#include "simple_king.h"


int pos = 0;
int ypos = window_height-100;
bool firstTime = true;
Bitmap* bmTree = nullptr;
Bitmap* bmPlayer = nullptr;
Bitmap* bmFont = nullptr;
Bitmap* bmGrassTile = nullptr;
Bitmap* bmWaterTile = nullptr;
Texture* texTree;
Texture* texUITiles;
Texture* fpsTexture;
Texture* amountTexture;
Texture* houseDiffuseTexture;
Texture* storageDiffuse;
Texture* treeDiffuse;
Texture* stoneAmountTexture;
Texture* goldAmountTexture;
Texture* citizenAmountTexture;

// Random number
using u32    = uint_least32_t;
using engine = std::mt19937;
engine generator;
u32 seed;
// end random number

class Citizen;
class StorageNode;
class BuildingData;


glm::vec3 normalizeHitPoint(glm::vec3 hitpoint);
void updateOccupiedRegister(glm::vec3 hitpoint);
BuildingData* findNearestStorageNode(Citizen* cit, StorageNode* excludedNode = nullptr);

int getRandomInt(int min, int max) {
    std::uniform_int_distribution< u32 > distribute( min, max );
    return distribute( generator );
}



void changeCitizenToFindNewStorageState(Citizen *pCitizen);




float getOccupationTimeForMoveTargetType(MoveTargetType moveTargetType) {
    if (moveTargetType == MoveTargetType::RandomWanderLocation) {
        return 3;
    } else if (moveTargetType == MoveTargetType::ResourceNode) {
        return 4;
    } else if (moveTargetType == MoveTargetType::StorageNode) {
        return 1;
    }
}

static GameState* gameState;

void spawnResourceNodes() {
    // Spawn some stone nodes.
    // Later the locations should come from a random noise function or similar.
    auto stoneNode1 = new ResourceNode();
    stoneNode1->resourceType = ResourceType::Stone;
    stoneNode1->position = glm::vec3{5, 0, -4};
    updateOccupiedRegister(stoneNode1->position);
    gameState->resourceNodes.push_back(stoneNode1);

    auto stoneNode2 = new ResourceNode();
    stoneNode2->resourceType = ResourceType::Stone;
    stoneNode2->position = glm::vec3{4, 0, -9};
    updateOccupiedRegister(stoneNode2->position);
    gameState->resourceNodes.push_back(stoneNode2);

    // Tree nodes
    for (int i = 0; i < 10; i++) {
        auto tn = new ResourceNode();
        tn->resourceType = ResourceType::Wood;
        tn->position = glm::vec3{4 + i, 0, -10 -i };
        updateOccupiedRegister(tn->position);
        gameState->resourceNodes.push_back(tn);
    }

}

void runUDPServer() {
    startSocketServer(9010);
    while (true) {
        receiveFromSocketServer();
    }
}

void init() {
    if (!testJson()) {
        exit(3);
    }

    std::thread udpServerThread(&runUDPServer);
    udpServerThread.detach();

    std::random_device os_seed;
    seed = os_seed();
    engine generator( seed );

    if (!testVectorMath()) exit(1);
    if (!testMatrixMath()) exit(1);

    printf("init game\n");
    fflush(stdout);
    ypos=10;
    enableVsync(false);
    loadBitmap("../assets/player.bmp", &bmPlayer);
    loadBitmap("../assets/tree.bmp", &bmTree);
    loadBitmap("../assets/font.bmp", &bmFont);
    loadBitmap("../assets/grass_tile_iso.bmp", &bmGrassTile);
    loadBitmap("../assets/water_tile_iso.bmp", &bmWaterTile);
    srand(time(NULL));

    texTree = createTextureFromFile("../assets/tree.bmp", ColorFormat::BGRA);
    texUITiles = createTextureFromFile("../assets/ui_tiles.bmp", ColorFormat::BGRA);
    houseDiffuseTexture = createTextureFromFile("../assets/house_diffuse2.png", ColorFormat::RGBA);
    storageDiffuse = createTextureFromFile("../assets/storage_diffuse.png", ColorFormat::RGBA);
    treeDiffuse = createTextureFromFile("../assets/tree_diffuse.png", ColorFormat::RGBA);

    gameState = new GameState();
    gameState->gamePlayCamera = new Camera();
    glm::vec3 camLoc = {4, 12, 2};
    gameState->gamePlayCamera->location = {camLoc.x, camLoc.y, camLoc.z};
    glm::vec3 lookAtTarget = camLoc + gameState->gamePlayLookFwd;
    gameState->gamePlayCamera->lookAtTarget = {lookAtTarget.x, lookAtTarget.y, lookAtTarget.z};
    gameState->gamePlayCamera->type = CameraType::OrthoGameplay;
    gameState->uiCamera = new Camera();
    gameState->uiCamera->location = {0, 0, 0};
    gameState->uiCamera->lookAtTarget = {0, 0, -1};
    gameState->uiCamera->type = CameraType::Ortho;
    for (int i = 0; i < 1000; i++) {
        gameState->occupiedRegister[i] = false;
    }

    // 3D Models
    gameState->cubeMesh = loadMeshFromFile("../assets/cube.obj");
    gameState->houseMesh = loadMeshFromFile("../assets/house.obj");
    gameState->barrackMesh = loadMeshFromFile("../assets/barrack.obj");
    gameState->islandMesh = loadMeshFromFile("../assets/island.obj");
    gameState->stoneNodeMesh = loadMeshFromFile("../assets/stone_node.obj");
    gameState->storageNodeMesh = loadMeshFromFile("../assets/storage_node.obj");
    gameState->treeNodeMesh = loadMeshFromFile("../assets/tree.obj");
    gameState->planeMesh = parseGLTF(parseJson(readFile("../assets/basic_block_human.gltf")));
    gameState->maleFarmerMesh = parseGLTF(parseJson(readFile("../assets/male_farmer.gltf")));


    gameState->textures["tree_diffuse"] = createTextureFromFile("../assets/tree_diffuse.png", ColorFormat::RGBA);
    gameState->textures["grass_rock"] = createTextureFromFile("../assets/grass1024.png", ColorFormat::RGBA);
    gameState->textures["farmer_male_diffuse"] = createTextureFromFile("../assets/farmer/farmer_male_diffuse.png", ColorFormat::RGBA);

    // Sound effects and music
    // These files might not be checked into Git.
    // If you miss them, you need to comment these lines (and other places where playSound is called
    // or use other sounds as placeholders.
    gameState->sounds["stomp"] = loadSoundFileExt("../assets/sound/stomp.wav");
    gameState->sounds["final_frontier"] = loadSoundFileExt("../assets/sound/final_frontier_bg.wav");
    //playSound(gameState->sounds["final_frontier"], false);

    // This is used for text rendering:
    fpsTexture = createTextTexture(192, 32);
    amountTexture = createTextTexture(64, 32);

    spawnResourceNodes();

}



ResourceNode* findNearestFreeResourceNode(Citizen* cit) {
    float nearestDistance = 10000;
    ResourceNode* nearestNode = nullptr;
    for (auto rn : gameState->resourceNodes) {
        float dist = glm::distance(rn->position, cit->position);
        if (dist < nearestDistance && rn->hasJob()) {
            nearestDistance = dist;
            nearestNode = rn;
        }
    }

    return nearestNode;

}

BuildingData* findNearestStorageNode(Citizen* cit, StorageNode* excludedNode) {
    float nearestDistance = 10000;
    BuildingData* nearestNode = nullptr;
    for (auto b : gameState->buildingData) {
        float dist = glm::distance(b->position, cit->position);
        if (dist < nearestDistance && b->storageNode) {
            if (!excludedNode || b->storageNode != excludedNode) {
                nearestDistance = dist;
                nearestNode = b;
            }
        }
    }
    return nearestNode;
}

glm::vec3 findRandomLocation() {
    int randX = getRandomInt(2, 2);
    int randZ = getRandomInt(4, 8);
    return glm::vec3(randX, 0, -randZ);
}

/**
 * The idea is that the current moveTargetType is the
 * actual last one of that citizen.
 * We assume we have been called to find a NEW target
 * and that the current MoveTargetType is the OLD target essentially.
 * Based on where the citizen was last, we deduct where it should go next,
 * following some rules.
 *
 * @param cit
 */
void findMoveTargetForCitizen(Citizen* cit) {

    if (cit->moveTargetType == MoveTargetType::RandomWanderLocation) {
        if (cit->resourcesCarriedAmount == 0) {
            auto nearestNode = findNearestFreeResourceNode(cit);
            if (nearestNode) {
                cit->hasMoveTarget = true;
                cit->nextMoveTargetPosition = nearestNode->position;
                cit->moveTarget = nearestNode;
                cit->jobState = CitizenJobState::Gatherer;
                cit->occupationState = CitizenOccupationState::Walking;
                cit->moveTargetType = MoveTargetType::ResourceNode;
                nearestNode->assignGatherer(cit);

            } else {
                auto loc = findRandomLocation();
                cit->hasMoveTarget = true;
                cit->moveTargetType = MoveTargetType::RandomWanderLocation;
            }
        } else {
            changeCitizenToFindNewStorageState(cit);
        }

    }

    else if (cit->moveTargetType == MoveTargetType::ResourceNode) {
        auto storageBuildingData = findNearestStorageNode(cit);
        if (storageBuildingData) {
            cit->hasMoveTarget = true;
            cit->nextMoveTargetPosition = storageBuildingData->position;
            cit->moveTarget = storageBuildingData;
            cit->jobState = CitizenJobState::Gatherer;
            cit->occupationState = CitizenOccupationState::Walking;
            cit->moveTargetType = MoveTargetType::StorageNode;
        } else {
            cit->hasMoveTarget = false;
            cit->moveTargetType = MoveTargetType::RandomWanderLocation;
        }
    } else if (cit->moveTargetType == MoveTargetType::StorageNode) {
        // We need to check if we still carry stuff.
        // This normally means our last storage node was full!
        // In this case we search for a better storage node.
        // Otherwise, if we are empty:
        // First assume we still have a resource node (the job).
        // So we want to go back there.
        // Depletion of the node would only be recognised when being there.
        if (cit->resourcesCarriedAmount > 0) {
            changeCitizenToFindNewStorageState(cit);
        }
        if (cit->jobState == CitizenJobState::Gatherer) {
            // We assume he is assigned to a resourceNode
            // TODO maybe create dedicated and typed job nodes, one for each type
            // so we can more safely check which is available.
            // This could kill the program easily at runtime here:
            if (cit->jobNode) {
                cit->hasMoveTarget = true;
                cit->moveTargetType = MoveTargetType::ResourceNode;
                cit->moveTarget = cit->jobNode;
                cit->nextMoveTargetPosition =((ResourceNode*)cit->jobNode)->position;
                cit->occupationState = CitizenOccupationState::Walking;
            }
        }

    }

}

void changeCitizenToFindNewStorageState(Citizen *cit) {
    BuildingData* bd = (BuildingData*) cit->moveTarget;
    auto storageNodeFull = bd->storageNode;
    auto storageBuildingData = findNearestStorageNode(cit, storageNodeFull);
    if (storageBuildingData) {
        cit->hasMoveTarget = true;
        cit->nextMoveTargetPosition = storageBuildingData->position;
        cit->moveTarget = storageBuildingData;
        cit->jobState = CitizenJobState::Gatherer;
        cit->occupationState = CitizenOccupationState::Walking;
        cit->moveTargetType = MoveTargetType::StorageNode;
    } else {
        cit->hasMoveTarget = false;
        cit->moveTargetType = MoveTargetType::RandomWanderLocation;
    }
}

void tickCitizens() {
    float ftSecs = (ftMicros / 1000.0f / 1000.0f);
    float citizenMoveSpeed = 1;
    for (auto cit : gameState->citizens) {
        if (!cit->hasMoveTarget) {
            findMoveTargetForCitizen(cit);
        }

        if (glm::distance(cit->position, cit->nextMoveTargetPosition) > 0.25) {
            glm::vec3 moveDir = glm::normalize(cit->nextMoveTargetPosition - cit->position);
            cit->position += (moveDir * citizenMoveSpeed * ftSecs);

        } else {
            // We have reached our next target. Check what the type of the target is.
            // If this is a resource node, go into gathering state.
            // If this is storage node, go into unloading state.
            // (or loading, depending, what you do, get or deliver).
            // If this is a random wander target, well, then search for another one.
            if (cit->moveTargetType == MoveTargetType::ResourceNode) {
                cit->occupationState = CitizenOccupationState::Gathering;
                cit->resourcesCarriedAmount = ((ResourceNode*)cit->moveTarget)->amountPerGathering;
                cit->lastOccupationStartTime += (ftMicros / 1000.0f / 1000.0f);
            }

            if (cit->moveTargetType == MoveTargetType::RandomWanderLocation) {
                cit->occupationState = CitizenOccupationState::Walking;
            }

            if (cit->moveTargetType == MoveTargetType::StorageNode) {
                StorageNode* storageNode =  ((BuildingData*)cit->moveTarget)->storageNode;
                // Does the storage place still have room
                if ((storageNode->amount + cit->resourcesCarriedAmount) < storageNode->maxStorage) {
                    cit->occupationState = CitizenOccupationState::Unloading;
                    storageNode->amount += cit->resourcesCarriedAmount;
                    cit->resourcesCarriedAmount = 0;
                    cit->lastOccupationStartTime += (ftMicros / 1000.0f / 1000.0f);
                } else {
                    changeCitizenToFindNewStorageState(cit);
                }

            }

            if (cit->lastOccupationStartTime >= getOccupationTimeForMoveTargetType(cit->moveTargetType)) {
                cit->lastOccupationStartTime = 0;
                findMoveTargetForCitizen(cit);

            }
        }

    }
}

void checkButtonHover() {
    float l = gameState->buildButtonsLeftOffset;

    if (mouse_x > l-16 && mouse_x < (l-16 + 32) &&
        mouse_y > 190-16 && mouse_y < (190-16 + 32)) {
        gameState->buttonHoverIndex = 0;

    } else if (mouse_x > l-16 && mouse_x < (l-16 + 32) &&
               mouse_y > 150-16 && mouse_y < (150-16 + 32)) {
        gameState->buttonHoverIndex = 1;
    }
    else if (mouse_x > l-16 && mouse_x < (l-16 + 32) &&
             mouse_y > 110-16 && mouse_y < (110-16 + 32)) {
        gameState->buttonHoverIndex = 2;
    }
    else {
        gameState->buttonHoverIndex = -1;
    }
}

/**
 * When placing a building we need to write the occupied cells
 * into the occupiedRegister.
 *
 * The occupied register goes from 0/0 to 100/100.
 * 0/0 is in the lower left corner.
 *
 * In worldspace coordinates, the upper right corner is 100/-100.
 * We need to transfrom this when checking for occupation and we get handed a world space
 * coordinate.
 *
 * @param hitpoint The place where the building was placed.
 */
void updateOccupiedRegister(glm::vec3 hitpoint) {
    // Currently buildings occupy always 4 places. The spots are located around
    // the hit point.
    int x = (int) hitpoint.x;
    int y = (int) hitpoint.z * -1;
    int lowerLeft = (x-1) + ((y-1)*100);
    int upperLeft = (x-1) + ((y)*100);
    int lowerRight = (x) + ((y-1)*100);
    int upperRight = (x) + ((y)*100);
    gameState->occupiedRegister[lowerLeft] = true;
    gameState->occupiedRegister[upperLeft] = true;
    gameState->occupiedRegister[lowerRight] = true;
    gameState->occupiedRegister[upperRight] = true;

    printf("lowerleft: %d\n", lowerLeft);
    printf("uppperleft: %d\n", upperLeft);
    printf("lowerRight: %d\n", lowerRight);
    printf("upperRight: %d\n", upperRight);

}

bool checkPlaceInOccupiedRegister(glm::vec3 hitpoint) {
    int x = (int) hitpoint.x;
    int y = (int) hitpoint.z * -1;
    //printf("x/y: %d/%d\n", x, y);
    int lowerLeft = (x-1) + ((y-1)*100);
    int upperLeft = (x-1) + ((y)*100);
    int lowerRight = (x) + ((y-1)*100);
    int upperRight = (x) + ((y)*100);

    return gameState->occupiedRegister[lowerLeft] ||
           gameState->occupiedRegister[upperLeft] ||
           gameState->occupiedRegister[lowerRight] ||
           gameState->occupiedRegister[upperRight] ;
}

void checkSwitchToSpecificBuildMode(int buildingIndex, BuildState buildState) {
    if (lbuttonUp && gameState->buttonHoverIndex == buildingIndex && gameState->buildState != buildState) {
        gameState->buildState = buildState;
        // We eat the event here,
        // as we do not want any other place in the code to interfere here.
        // This is the most prioritary sequence.
        lbuttonUp = false;
    }
}



/**
 * We move the camera with the WASD keys
 * and potentially later with a gamepad right stick.
 *
 */
void checkCameraMovement() {
    float camspeed = 0.007;
    float dir = 0;
    float hdir = 0;
    if (isKeyDown('W')) {
        dir = 1;
    }
    if (isKeyDown('S')) {
        dir = -1;
    }

    if (isKeyDown('A')) {
        hdir = -1;
    }
    if (isKeyDown('D')) {
        hdir = 1;
    }


    auto fwd = camFwd(gameState->gamePlayCamera) ;
    auto right = cameraRightVector(gameState->gamePlayCamera);
    glm::vec3 loc = toGlm(gameState->gamePlayCamera->location);
    loc += glm::vec3{camspeed * fwd.x, 0, camspeed * fwd.z} * dir;
    loc += glm::vec3{camspeed * right.x * 0.33  , 0, camspeed * right.z * 0.33} * hdir;
    auto tgt = loc + glm::vec3{fwd.x, fwd.y, fwd.z};
    gameState->gamePlayCamera->location = {loc.x, loc.y, loc.z};
    gameState->gamePlayCamera->lookAtTarget = {tgt.x, tgt.y, tgt.z};

}

void checkEscape() {
    if (keyPressed(VK_ESCAPE))
    {
        gameState->shouldRun = false;
    }
}

void spawnCitizens() {

    for (int i = 0; i < gameState->buildingData.size(); i++) {
        auto building = gameState->buildingData[i];
        building->lastUnitSpawnTime += (ftMicros / 1000.0f / 1000.0f);
        if (building->spawnsInGeneral() && building->lastUnitSpawnTime > 5 && building->unitsSpawned < building->maxUnitsToSpawn) {
            Citizen* citizen = new Citizen();
            citizen->position = building->position + glm::vec3 {1, 0, 0};
            gameState->citizens.push_back(citizen);
            building->unitsSpawned++;
            building->lastUnitSpawnTime = 0;
            gameState->resources.citizens++;
        }
    }

}


void processSocketMessage() {
    auto sockMessage = getLatestSocketMessage();
    if (!sockMessage.empty()) {
        if (sockMessage == "stop") {
            gameState->shouldRun = false;
        }
    }
}

void update() {
    processSocketMessage();
    checkEscape();
    checkCameraMovement();
    checkButtonHover();
    pos++;

    auto ray = initRayWithScreenCoordinates(gameState->gamePlayCamera);
    glm::vec3 normalizedLocation;
    if (rayIntersectsPlane(ray, {0, 1, 0}, {1, 0, 0}, &gameState->lastRayHitPoint)) {
        // printf("hit %f/%f/%f\n", gameState->lastRayHitPoint.x, gameState->lastRayHitPoint.y, gameState->lastRayHitPoint.z);
        //normalizedLocation = glm::vec3({(float)(int)rh.x, 0, (float)(round(rh.z))});
        normalizedLocation = normalizeHitPoint(gameState->lastRayHitPoint);
    }

    if (gameState->buildState != BuildState::None) {
        gameState->placementAllowed = true;
        if (gameState->lastRayHitPoint.x < 1) gameState->placementAllowed = false;
        if (gameState->lastRayHitPoint.z > -0.5) gameState->placementAllowed = false;
        if (checkPlaceInOccupiedRegister(normalizedLocation)) gameState->placementAllowed = false;
    }

    checkSwitchToSpecificBuildMode(0, BuildState::House);
    checkSwitchToSpecificBuildMode(1, BuildState::Barrack);
    checkSwitchToSpecificBuildMode(2, BuildState::Storage);


    if (lbuttonUp && gameState->buildState != BuildState::None && gameState->placementAllowed) {
        printf("normalized loc: %f/%f/%f\n", normalizedLocation.x, normalizedLocation.y, normalizedLocation.z);
        updateOccupiedRegister(normalizedLocation);
        BuildingData* bd = new BuildingData();
        bd->position = normalizedLocation;
        bd->buildState = gameState->buildState;
        if (gameState->buildState == BuildState::Storage) {
            bd->storageNode = new StorageNode();
        }
        gameState->buildingData.push_back(bd);
        gameState->resources.wood -= 25;
        gameState->resources.stone -= 35;
        gameState->resources.gold -= 12;

        gameState->buildState = BuildState::None;
        playSound(gameState->sounds["stomp"], false);
    }

    spawnCitizens();
    tickCitizens();

}

void drawTerrainChunk(int startX, int startY) {

    int num = 8;
    int width = 1;
    int height = 16;
    static int ycount = 0;
    for (int i = 0; i < num; i++) {
        for (int x = 0; x < width; x++) {
            ycount = 0;
            for (int y = height; y > 0; y--) {
                auto xcoord = (startX - 16*i) + x*34 + (ycount * 16);
                auto ycoord = y*8 + (startY-8*i);
                if (y % 8 == 0 ) {
                    ycoord = ycoord - 4;
                    drawBitmap(xcoord, ycoord, bmWaterTile);

                } else {
                    drawBitmap(xcoord, ycoord, bmGrassTile);
                }

                ycount++;
            }
        }
    }
}



Mesh* selectMeshForBuildState(BuildState buildState) {
    if (buildState == BuildState::House) {
        return gameState->houseMesh;
    }
    else if (buildState == BuildState::Barrack) {
        return gameState->barrackMesh;
    }
    else if (buildState == BuildState::Storage) {
        return gameState->storageNodeMesh;
    }

}

glm::vec3 normalizeHitPoint(glm::vec3 hitpoint) {
    auto rh = hitpoint;
    return {(float)(int)rh.x, 0, (float)(round(rh.z))};
}

static void renderStorageContents(BuildingData* storageBuilding) {
    auto stn = storageBuilding->storageNode;
    int rows = ceil((float)stn->amount / 70.0f);
    float scaleFactor = stn->amount * 0.1f;

    bindMesh(gameState->cubeMesh);
    scale({0.03f * scaleFactor , 0.2f * scaleFactor , 0.04f * scaleFactor });
    glm::vec3 pos = storageBuilding->position + glm::vec3(0, 0.5, 0);
    location({pos.x, pos.y, pos.z });
    foregroundColor({0.3, 0.3, 0.3, 1});
    drawMesh();









}

static void renderBuildings() {
    // Draw all existing houses
    for (auto b :  gameState->buildingData) {
        scale({2, 4,2});
        if (b->buildState == BuildState::House) {
            bindTexture(houseDiffuseTexture);
            flipUvs(true);
        }
        else if (b->buildState== BuildState::Storage) {
            bindTexture(storageDiffuse);
            flipUvs(true);
        }
        else {
            foregroundColor({0.9, 0.9, 0.8, 1});
        }

        bindMesh(selectMeshForBuildState(b->buildState));
        auto rh = gameState->lastRayHitPoint;
        auto loc = b->position;
        location({loc.x, 0, loc.z});
        drawMesh();

        if (b->buildState == BuildState::Storage) {
            renderStorageContents(b);
        }
    }
    bindTexture(nullptr);
}

static void renderResourceNodes() {
    for (auto rn : gameState->resourceNodes) {
        scale({2, 3,2});
        foregroundColor({0.9, 0.9, 0.8, 1});
        if (rn->resourceType == ResourceType::Stone) {
            bindMesh(gameState->stoneNodeMesh);
        } else if (rn->resourceType == ResourceType::Wood){
            bindMesh(gameState->treeNodeMesh);
            bindTexture(treeDiffuse);
        }
        auto rh = gameState->lastRayHitPoint;
        location({rn->position.x, rn->position.y-0.25f, rn->position.z});
        drawMesh();
    }
}

// Draw the dragged build mesh if we are in the build state
static void renderDraggedBuilding() {

    if (gameState->buildState != BuildState::None) {

        scale({2, 3,2});
        if (gameState->placementAllowed) {
            foregroundColor({0.1, 0.9, 0.1, 0.5});
        } else {
            foregroundColor({0.9, 0.2, 0.2, 0.5});
        }
        bindMesh(selectMeshForBuildState(gameState->buildState));
        auto hp = normalizeHitPoint(gameState->lastRayHitPoint);
        location({hp.x, hp.y, hp.z});

        drawMesh();
    }
}

static void renderCitizens() {
    for (int i = 0; i < gameState->citizens.size(); i++) {
        auto cit = gameState->citizens[i];
        scale({.4, .4,.4});
        foregroundColor({0.8, 0.4, 0.1, 1});
        if (cit->jobState == CitizenJobState::Jobless) {
            foregroundColor({0.9, 0.1, 0.1, 1});
        }
        if (cit->occupationState == CitizenOccupationState::Gathering) {
            foregroundColor({0.2, 0.2, 0.8, 1});
        }
        bindTexture(gameState->textures["farmer_male_diffuse"]);
        flipUvs(false);
        bindMesh(gameState->maleFarmerMesh);
        location({cit->position.x, cit->position.y, cit->position.z});
        drawMesh();
        // Draw the stuff the citizen is carrying.
        if (cit->resourcesCarriedAmount > 0) {
            bindMesh(gameState->cubeMesh);
            scale({0.03, 0.2, 0.04});
            foregroundColor({0.3, 0.3, 0.3, 1});
            location({cit->position.x, cit->position.y + 0.5f, cit->position.z-0.1f});
            drawMesh();
        }

    }
}


void render() {

    bindCamera(gameState->gamePlayCamera);

    scale({1, 1,1});
    foregroundColor({0.3, 0.6, 0.2, .1});
    location({0, 0, 0});
    //   drawGrid();

    // Border of world
    lightingOn();

    // Draw the main island
    bindTexture(gameState->textures["grass_rock"]);
    bindMesh(gameState->islandMesh);
    scale({10, 10,10});
    //foregroundColor({0.2, 0.6, 0.2, 1});
    location({10, 0, -10});
    uvScale(3);
    drawMesh();
    bindTexture(nullptr);
    uvScale(1);

    renderDraggedBuilding();
    renderResourceNodes();
    renderBuildings();
    renderCitizens();

    // Draw our gtlf mesh
    bindMesh(gameState->planeMesh);
    bindTexture(nullptr);
    foregroundColor({.8, .4, 0, 1});
    location({10, 0.5, -3});
    scale({.5, .5, .5});
    // Rotate if this is a plane to better/at all see it.
    rotation({0, 0, 0});
    drawMesh();
    rotation({0, 0, 0});

    // UI
    lightingOff;
    bindCamera(gameState->uiCamera);

    bindTexture(texUITiles);
    tilingOn(true);
    scale({32,32 ,1 });
    tileData(0, 0, 32, 32);
    float btnLeft = window_width - 40;

    location({btnLeft, 190, -1});
    drawPlane();
    if (gameState->buttonHoverIndex == 0) {
        tileData(5, 0, 32, 32);
        location({btnLeft, 190, -0.95});
        drawPlane();
    }

    tileData(1, 0, 32, 32);
    location({btnLeft, 150, -1});
    drawPlane();
    if (gameState->buttonHoverIndex == 1) {
        tileData(5, 0, 32, 32);
        location({btnLeft, 150, -0.95});
        drawPlane();
    }

    tileData(2, 0, 32, 32);
    location({btnLeft, 110, -1});
    drawPlane();
    if (gameState->buttonHoverIndex == 2) {
        tileData(5, 0, 32, 32);
        location({btnLeft, 110, -0.95});
        drawPlane();
    }

    {
        // Resource info background plane
        float rtop = window_height-16;
        scale({400, 32, 1});
        tileData(6, 0, 32, 32);
        location({window_width / 2, rtop, -2});
        drawPlane();

        // Draw wood, stone and gold resource icons
        scale({24, 24, 1});
        tileData(1, 1, 32, 32);
        location({window_width/2 - 180, rtop, -1});
        drawPlane();
        tileData(2, 1, 32, 32);
        location({window_width/2 - 100, rtop, -1});
        drawPlane();
        tileData(3, 1, 32, 32);
        location({window_width/2 - 20, rtop, -1});
        drawPlane();
        tileData(4, 1, 32, 32);
        location({window_width/2 + 60, rtop, -1});
        drawPlane();

        tilingOn(false);

        // Resource amount rendering
        foregroundColor({.1, 0.1, .1, 1});
        updateAndDrawText(std::to_string(gameState->resources.wood).c_str(), amountTexture, window_width/2 - 130, rtop - 8);
        updateAndDrawText(std::to_string(gameState->resources.stone).c_str(), amountTexture, window_width/2 - 50, rtop-8);
        updateAndDrawText(std::to_string(gameState->resources.gold).c_str(), amountTexture, window_width/2 + 30, rtop-8);
        updateAndDrawText(std::to_string(gameState->resources.citizens).c_str(), amountTexture, window_width/2 + 110, rtop-8);
    }



    // FPS Text rendering
    foregroundColor({1, 1, 1, 1});
    char buf[100];
    sprintf_s(buf, 100, "FT:%0.0fmcs %d/%d", ftMicros, mouse_x, mouse_y);
    updateAndDrawText(buf, fpsTexture, fpsTexture->bitmap->width /2 + 2, window_height-30);


}



bool gameLoop() {
    if (firstTime) {
        init();
        firstTime=false;
    }
    update();
    render();

    return gameState->shouldRun;
}


/**
 * This uses the non opengl methods.
 * We render directly in software to a texture
 * which is presented fullscreen at the end of the frame.
 */
#ifdef RENDER2D
void renderIn2DMode() {
    // This draws a small red square which moves horizontall
//	for (int i = pos; i < pos+16; i++) {
//		for (int y = ypos; y < ypos+16;y++) {
//			setPixel(i, y, 255, 2, 2, 255);
//		}
//	}

    drawBitmap(0, 584, bmTree);

//    for (int t = 0; t < 40; t++) {
//		drawBitmap(t * 18, 300, bmTree);
//	}
    drawBitmapTile(5, 200, 8, 2, 0, bmFont);

    drawTerrainChunk(428, 264);
    drawTerrainChunk(300, 200);
    drawTerrainChunk(164+8, 132+4);

    drawBitmap(116, 208, bmGrassTile);
    drawBitmap(100, 200, bmGrassTile);

    drawBitmap(300, 500, bmGrassTile);
    drawBitmap(316, 492, bmGrassTile);
    drawBitmap(300, 492-8, bmGrassTile);
    drawBitmap(300-16, 492-16, bmGrassTile);
    drawBitmap(300, 492-24, bmGrassTile);
    drawBitmap(316, 492-32, bmGrassTile);
    drawBitmap(332, 492-40, bmGrassTile);

    drawBitmap(312, 485, bmPlayer);

    char buf[100];
    sprintf_s(buf, 100, "FT: %0.0f micros gj", ftMicros);
    drawText(buf, 16, 584, bmFont);
}
#endif