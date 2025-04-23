#include <stdio.h>
#include <thread>
#include <chrono>
#include <functional>
#include <queue>
#include <mutex>
#include <vector>
#include <Recast.h>
#include <DetourNavMesh.h>
#include <DetourNavMeshBuilder.h>
#include <DetourCommon.h>
#include "FirstPersonController.h"
#include "mtking.h"


using namespace mtk;
using namespace  std;

mutex myMutex;

namespace mtking {

    class TreeNode {
    public:
        TreeNode(const std::string& name, void* data) {
            this->_name = name;
            this->_data = data;
        }

    private:
        std::string _name;
        void* _data = nullptr;
        TreeNode* _left = nullptr;
        TreeNode* _right = nullptr;
    };

    class Tree {
    public:
        Tree(TreeNode* root) {
            this->_root = root;
        }

    private:
        TreeNode* _root = nullptr;
    };
}


struct Job{
    std::function<void(void*)> function;
    void* data = nullptr;
};

std::queue<Job> jobQueue;

GameState* gameState = nullptr;

/**
 * Test our job pool implementation
 */
void pushTestJobs() {

    static int counter = 0;
    counter++;
    std::function<void(void*)> task = [](void* arg){
        int c = *(int*) (arg);
        printf("hello from task: %d \n", c);
    };
    static int payload = 0;
    payload++;
    int* data = new int(payload);
    lock_guard<mutex> lock(myMutex);
    jobQueue.push(Job{task, data});

}

void updateMovement() {
    gameState->fpsController->updateMovement();
}


void update(){
    auto ftSecs = (float)ftMicros / 1000.0 / 1000.0;
    if (keyPressed(VK_ESCAPE))
    {
        gameState->shouldRun = false;
    }
    if (keyPressed('E')) {
        printf("e pressed");
    }

    auto button = gameState->uiSystem->queryHovered(mouse_x, mouse_y);
    if (button) {
        button.value().scaled(1.25);
    } else {
        // Draw everything normal
        for (auto& b : gameState->uiSystem->buttons()) {
            b.unscaled();
        }
    }

    updateMovement();




#ifdef MASSTEST
    /**
     * Add a sprite periodically.
     * This is of course dumb in itself and just for demonstration.
     */
    {
        static double totalTime = 0;
        totalTime += ftSecs;
        static int count = 0;
        if (totalTime > 4) {
            count++;
            //gameState->spriteBatch->addSprite({10 + (count * 10), 600}, {16 + (count), 16 + count});

            totalTime = 0;
        }
    }

    // Simulate the updating of several objects
    static double updateTimeTotal = 0;
    updateTimeTotal += ftSecs;
    {
        int index = 0;
        for (auto& go : *gameState->gameObjects) {

            if (index % 10 == 0) {
                go.screenPos.x += 0.01;
                go.screenPos.y += 0.02;
            }

            if (index % 56 == 0) {
                go.screenPos.x -= 0.02;
                go.screenPos.y += 0.01;
            }

            if (index % 23 == 0) {
                go.sizeInPixels.x = 16;
                go.sizeInPixels.y = 16;
            }

            if (updateTimeTotal > 5) {
                if (index % 8 == 0) {
                    go.screenPos.x -= 0.02;
                    go.screenPos.y += 0.01;
                }
            }

            if (updateTimeTotal > 9) {
                if (index % 4 == 0) {
                    go.screenPos.x -= 0.05;
                    go.screenPos.y += 0.02;
                }
            }

            if (updateTimeTotal > 13) {
                if (index % 3 == 0) {
                    go.screenPos.x += 0.03;
                    go.screenPos.y -= 0.04;
                }
            }

            if (updateTimeTotal > 16) {
                if (index % 2 == 0) {
                    go.screenPos.x -= 0.01;
                    go.screenPos.y -= 0.03;
                }
            }

            gameState->spriteBatch->updateSprite(index++, go.screenPos, go.sizeInPixels);
        }
    }
#endif


}

void worker(int id) {
    while (true) {
        this_thread::sleep_for(chrono::milliseconds(10));

        {
            lock_guard<mutex> lock(myMutex);
            printf("worker received lock: %d\n", id);
            if (!jobQueue.empty()) {
                auto job = (Job) jobQueue.front();
                jobQueue.pop();
                job.function(job.data);
                if (job.data != nullptr) {
                    delete(job.data);
                }

            }
        }
    }
}

void initThreadPool() {
    for (int i = 0; i < 6; i++) {
        auto w = std::thread(worker, i);
        w.detach();
    }

}

void initNavMesh() {
    // We use Recast here for the moment.
    auto ctx = new rcContext();
    const int w = 1024;
    const int h = 1024;


    rcConfig cfg;
//    memset(&cfg, 0, sizeof(cfg));
//    cfg.cs = 0.3f; // Cell size
//    cfg.ch = 0.2f; // Cell height
//    cfg.walkableSlopeAngle = 45.0f;
//    cfg.walkableHeight = (int)ceilf(2.0f / cfg.ch); // Agent height / cell height
//    cfg.walkableClimb = (int)floorf(0.9f / cfg.ch); // Agent max climb / cell height
//    cfg.walkableRadius = (int)ceilf(0.6f / cfg.cs); // Agent radius / cell size
//    cfg.maxEdgeLen = (int)(12.0f / cfg.cs); // Max edge length
//    cfg.maxSimplificationError = 1.3f;
//    cfg.minRegionArea = (int)rcSqr(8); // Min region area
//    cfg.mergeRegionArea = (int)rcSqr(20); // Merge region area
//    cfg.maxVertsPerPoly = 6;
//    cfg.detailSampleDist = cfg.cs * 6.0f;
//    cfg.detailSampleMaxError = cfg.ch * 1.0f;
//
//    // Small plane for testing
//    static float vertices[] = { -10, 0, -10, 10, 0, -10, 10, 0, 10, -10, 0, 10 };
//    static int indices[] = { 0, 1, 2, 2, 3, 0 };
//    auto vertCount = sizeof(vertices) / sizeof(vertices[0]) / 3;
//    auto triCount = sizeof(indices) / sizeof(indices[0]) / 3;
//
//    rcVcopy(cfg.bmin, vertices);
//    rcVcopy(cfg.bmax, vertices);
//    for (int i = 1; i < vertCount; ++i) {
//        rcVmin(cfg.bmin, &vertices[i * 3]);
//        rcVmax(cfg.bmax, &vertices[i * 3]);
//    }
//    rcCreateHeightfield(ctx, *heightField, w, h, cfg.bmin, cfg.bmax, cfg.cs, cfg.ch);
//
//    // Step 4: Rasterize triangles into the heightfield
//    unsigned char* triAreas = new unsigned char[triCount];
//    rcMarkWalkableTriangles(ctx, cfg.walkableSlopeAngle, vertices, vertCount, indices, triCount, triAreas);
//    rcRasterizeTriangles(ctx, vertices, triAreas, triCount, *heightField, 1);
//    delete[] triAreas;
//
//    rcFilterLowHangingWalkableObstacles(ctx, cfg.walkableClimb, *heightField);
//    rcFilterLedgeSpans(ctx, cfg.walkableHeight, cfg.walkableClimb, *heightField);
//    rcFilterWalkableLowHeightSpans(ctx, cfg.walkableHeight, *heightField);
//
//    rcCompactHeightfield* compactHeightfield = rcAllocCompactHeightfield();
//    if (!compactHeightfield) {
//        std::cerr << "Failed to allocate compact heightfield" << std::endl;
//        exit(1);
//    }
//
//    rcBuildCompactHeightfield(ctx, cfg.walkableHeight, cfg.walkableClimb, *heightField, *compactHeightfield);
//    rcFreeHeightField(heightField);
//    rcErodeWalkableArea(ctx, cfg.walkableRadius, *compactHeightfield);
//    rcBuildDistanceField(ctx, *compactHeightfield);
//    rcBuildRegions(ctx, *compactHeightfield, 0, cfg.minRegionArea, cfg.mergeRegionArea);
//    // Step 7: Create the contour set
//    rcContourSet* contourSet = rcAllocContourSet();
//    if (!contourSet) {
//        std::cerr << "Failed to allocate contour set" << std::endl;
//        exit(1);
//    }
//    rcBuildContours(ctx, *compactHeightfield, cfg.maxSimplificationError, cfg.maxEdgeLen, *contourSet);
//
//    // Step 8: Build the polygon mesh from the contours
//    rcPolyMesh* polyMesh = rcAllocPolyMesh();
//    if (!polyMesh) {
//        std::cerr << "Failed to allocate poly mesh" << std::endl;
//        exit(-1);
//    }
//    rcBuildPolyMesh(ctx, *contourSet, cfg.maxVertsPerPoly, *polyMesh);
//    rcPolyMeshDetail* polyMeshDetail = rcAllocPolyMeshDetail();
//    if (!polyMeshDetail) {
//        std::cerr << "Failed to allocate poly mesh detail" << std::endl;
//        exit(-1);
//    }
//
//    rcBuildPolyMeshDetail(ctx, *polyMesh, *compactHeightfield, cfg.detailSampleDist, cfg.detailSampleMaxError, *polyMeshDetail);
//    rcFreeCompactHeightfield(compactHeightfield);
//    rcFreeContourSet(contourSet);
//
//    dtNavMeshCreateParams params;
//    memset(&params, 0, sizeof(params));
//    params.verts = polyMesh->verts;
//    params.vertCount = polyMesh->nverts;
//    params.polys = polyMesh->polys;
//    params.polyAreas = polyMesh->areas;
//    params.polyFlags = polyMesh->flags;
//    params.polyCount = polyMesh->npolys;
//    params.nvp = polyMesh->nvp;
//    params.detailMeshes = polyMeshDetail->meshes;
//    params.detailVerts = polyMeshDetail->verts;
//    params.detailVertsCount = polyMeshDetail->nverts;
//    params.detailTris = polyMeshDetail->tris;
//    params.detailTriCount = polyMeshDetail->ntris;
//    params.walkableHeight = cfg.walkableHeight;
//    params.walkableRadius = cfg.walkableRadius;
//    params.walkableClimb = cfg.walkableClimb;
////    params.bmin = cfg.bmin;
////    params.bmax = cfg.bmax;
//    params.cs = cfg.cs;
//    params.ch = cfg.ch;
//    params.buildBvTree = true;

}

std::map<std::string, Mesh*> loadMeshes() {
    std::map<std::string, Mesh*> meshes;
    meshes["arena"] = MeshImporter().importStaticMesh("../assets/smart_cube.glb");
    meshes["saloon"] = MeshImporter().importStaticMesh("../assets/saloon.glb");
    meshes["skyscraper1"] = MeshImporter().importStaticMesh("../assets/skyscraper1.glb");
    meshes["ground"] = MeshImporter().importStaticMesh("../assets/ground.glb");
    meshes["mountains1"] = MeshImporter().importStaticMesh("../assets/mountains1.glb");
    meshes["lpgun"] = MeshImporter().importStaticMesh("../assets/lp_gun.glb");
    meshes["skybox"] = MeshImporter().importStaticMesh("../assets/skybox.glb");

    return meshes;

}



void init() {
    gameState = new GameState();

    // Camera setup
    {
        gameState->gamePlayCamera = new Camera();
        glm::vec3 camLoc = {0, 1.8, 40};
        gameState->gamePlayCamera->location = {camLoc.x, camLoc.y, camLoc.z};
        glm::vec3 lookAtTarget = glm::vec3(0, 1.8, 0);
        gameState->gamePlayCamera->lookAtTarget = {lookAtTarget.x, lookAtTarget.y, lookAtTarget.z};
        gameState->gamePlayCamera->type = CameraType::Perspective;

        gameState->gunCamera = new Camera();
        camLoc = {0, 1.8, 0};
        gameState->gunCamera->location = {camLoc.x, camLoc.y, camLoc.z};
        lookAtTarget = glm::vec3(0, 1.8, -1);
        gameState->gunCamera->lookAtTarget = {lookAtTarget.x, lookAtTarget.y, lookAtTarget.z};
        gameState->gunCamera->type = CameraType::Perspective;



        gameState->uiCamera = new Camera();
        gameState->uiCamera->location = {0, 0, 0};
        gameState->uiCamera->lookAtTarget = {0, 0, -1};
        gameState->uiCamera->type = CameraType::Ortho;

        gameState->shadowMapCamera = new Camera();
        auto shadowCamLoc = glm::vec3(0, 12, -4) ;
        gameState->shadowMapCamera->location =  {shadowCamLoc.x, shadowCamLoc.y, shadowCamLoc.z};
        gameState->shadowMapCamera->lookAtTarget = {-5, 0, -6};
        gameState->shadowMapCamera->type = CameraType::OrthoGameplay;
    }

    // Character setup
    {
        gameState->character = new mtking::Character{{0, 0, 0}};
        gameState->fpsController = new mtking::FirstPersonController(gameState->character, gameState->gamePlayCamera);
    }

    gameState->uiSystem = new gru::UISystem();

    Bitmap* bm = nullptr;
    gameState->texTree = createTextureFromFile("../assets/tree.bmp", ColorFormat::BGRA);
    gameState->texTileMap = createTextureFromFile("../assets/number_tiles.png", ColorFormat::BGRA);
    gameState->baseTexture = createTextureFromFile("../assets/base_texture.png", ColorFormat::BGRA);
    gameState->gunTexture = createTextureFromFile("../assets/gun_tex.png", ColorFormat::RGBA);
    gameState->skyBoxTexture = createCubeMapTextureFromFile("../assets/skybox", ColorFormat::RGBA);

    loadBitmap("../assets/font.bmp", &gameState->font);
    gameState->fpsTexture = createTextTexture(512, 32);

    int nrOfSpritesMax = 5000;

    gameState->spriteBatch = new gru::SpriteBatch(nrOfSpritesMax);
    gameState->uiSpriteBatch = new gru::SpriteBatch(100);


    // Init UI Buttons
    gameState->uiSystem->registerSpriteBatch(gameState->uiSpriteBatch);
    auto tileForStartButton = new gru::TextureTile({0,0}, {32*3, 32*2}, {512, 512});
    auto tileForExitButton = new gru::TextureTile({32*3,0}, {2*32*3, 32*2}, {512, 512});
    gameState->uiSystem->registerButton({64, window_height - 200}, {128, 64}, "start_button", tileForStartButton);
    gameState->uiSystem->registerButton({64, window_height - 280}, {128, 64}, "exit_button", tileForExitButton);

    // Init crosshair
    auto tileForCrosshair = new gru::TextureTile({286, 16}, {320, 48}, {512, 512});
    gameState->spriteBatch->addSprite({window_width/2 ,window_height/2}, {16, 16}, tileForCrosshair);

    initNavMesh();

    gameState->meshes = loadMeshes();


// Reserve memory for 1000 game objects
#define MASSTEST_
#ifdef MASSTEST
    {
        gameState->gameObjects = new std::vector<GameObject>(nrOfSpritesMax);
        int col = 50;
        int row = 50;
        for (auto& go : *gameState->gameObjects) {

            if (col > 1100) {
                col = 50;
                row += 40;
            }
            if (row > 600) {
                row = 50;
            }

            go.screenPos.x = col ;
            go.screenPos.y = row;
            go.sizeInPixels = {32, 32};

            col +=40;

            gameState->spriteBatch->addSprite(go.screenPos,go.sizeInPixels, nullptr);
        }
    }
#endif
}


void renderScene() {
    bindCamera(gameState->gamePlayCamera);
    bindShadowMapCamera(gameState->shadowMapCamera);


    // Skybox
    {
        bindSkyboxTexture(gameState->skyBoxTexture);
        scale({1, 1, 1});
        bindMesh(gameState->meshes["skybox"]);
        //location(glm::vec3(0, 0, 0));
        lightingOff();
        shadowOff();
//        glDepthFunc(GL_LEQUAL);
//        glDepthMask(GL_FALSE);
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        drawMesh();
//        glDepthFunc(GL_LESS);
//        glDepthMask(GL_TRUE);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        bindSkyboxTexture(nullptr);

    }



    // Grid
    {
        scale({1, 1,1});
        foregroundColor({0.3, 0.6, 0.2, .1});
        location(glm::vec3{0, 0, 0});
        gridLines(100);
        drawGrid();
    }



    lightingOn();
    shadowOn();

    bindTexture(gameState->baseTexture);
    scale({1, 1, 1});
    bindMesh(gameState->meshes["arena"]);
    location(glm::vec3(30, 2, 0));
    drawMesh();


    // Render saloons
    {
        bindMesh(gameState->meshes["saloon"]);
        location(glm::vec3(-10, 1.1, 0));
        rotation({0, 90, 0});
        drawMesh();
    }

    // Draw some skyscrapers
    {
        bindMesh(gameState->meshes["skyscraper1"]);
        location(glm::vec3(-10, 0, -15));
        scale({1, 0.5, 1});
        rotation({0, 90, 0});
        drawMesh();

        scale({0.4, 0.7, 0.7});
        location(glm::vec3(30, 0, -5));
        drawMesh();

        scale({0.7, 0.8, 0.5});
        location(glm::vec3(10, 0, -25));
        drawMesh();

    }

    // Ground
    bindMesh(gameState->meshes["ground"]);
    location(glm::vec3(-10, 0, -15));
    scale({100, 10, 100});
    rotation({0, 0, 0});
    drawMesh();

    // Mountains
    {
        bindMesh(gameState->meshes["mountains1"]);
        location(glm::vec3(-150, 0, -225));
        scale({100, 100, 100});
        rotation({0, -30, 0});
        drawMesh();

        location(glm::vec3(-0, 0, -235));
        scale({150, 80, 130});
        rotation({0, 45, 0});
        drawMesh();

        location(glm::vec3(100, 0, -235));
        scale({90, 100, 100});
        rotation({0, 75, 0});
        drawMesh();

        location(glm::vec3(-30, 0, -435));
        scale({150, 150, 150});
        rotation({0, 85, 0});
        drawMesh();

        location(glm::vec3(-50, 0, -435));
        scale({150, 150, 150});
        rotation({0, 15, 0});
        drawMesh();

    }

    // Render fps weapon
    {
        bindMesh(gameState->meshes["lpgun"]);
        bindCamera(gameState->gunCamera);
        bindTexture(gameState->gunTexture);
        glDisable(GL_DEPTH_TEST);
        location(glm::vec3(1, 0.5, -2));
        scale({1, 1, 1});
        rotation({0, 0, 0});
        drawMesh();
        glEnable(GL_DEPTH_TEST);
    }

    // Reset everything so we do not leak state into the ui.
    // This is not ideal of course.
    // (Having to do this).
    rotation({0, 0,0});

}

void renderUI() {
    lightingOff();
    bindCamera(gameState->gamePlayCamera);
    bindTexture(gameState->texTileMap);

    bindCamera(gameState->uiCamera);

    gameState->spriteBatch->render();
    gameState->uiSpriteBatch->render();

    // FPS rendering
    {
        foregroundColor({.1, .7, .1, 1});
        char buf[160];
        sprintf_s(buf, 160, "FT:%6.1fmcs %4d/%4d",
                  ftMicros, mouse_x, mouse_y);
        font(gameState->font);
        updateAndDrawText(buf, gameState->fpsTexture, gameState->fpsTexture->bitmap->width / 2 + 2,
                          5);
    }

}


void render() {
    renderScene();
    renderUI();
}

bool gameLoop() {
    static bool initalized = false;
    if (!initalized) {
        init();
        initalized = true;
    }

    update();
    render();

    return gameState->shouldRun;
}