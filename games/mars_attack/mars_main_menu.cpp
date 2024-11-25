//
// Created by mgrus on 12.01.2024.
//
#include "mars_main_menu.h"
#include "mars_attack.h"
#include <graphics.h>

extern GameState* gameState;

int menuIndex = 0;

void initMainMenu() {
    gameState->textures["banner"] = createTextureFromFile("../assets/banner_image.png");
    gameState->textures["menu_items"] = createTextureFromFile("../assets/menu_items.png");
}

void updateMainMenu() {
    static bool firstTime = true;
    if (firstTime) {
        firstTime = false;
        initMainMenu();
    }

    if (keyPressed(VK_ESCAPE))
    {
        gameState->shouldRun = false;
    }

    if (keyPressed(VK_DOWN))
    {
        menuIndex ++;
        if (menuIndex > 2) {
            menuIndex = 2;
        }
    }

    if (keyPressed(VK_UP))
    {
        menuIndex--;
        if (menuIndex < 0) {
            menuIndex = 0;
        }
    }

    if (keyPressed(VK_RETURN))
    {
        if (menuIndex == 0) {
            gameState->level = Level::Gameplay;
        }

        if (menuIndex == 2) {
            gameState->shouldRun = false;
        }
    }
}

void tintRed(){
    tint(glm::vec4{0.8, 0, 0, 1});
}
void tintWhite(){
    tint(glm::vec4{1, 1, 1, 1});
}

void renderMainMenu() {
    bindCamera(gameState->uiCamera);
    lightingOff();

    bindTexture(gameState->textures["banner"]);
    flipUvs(true);
    location({window_width/2, 600, -1});
    scale({1280, 256, 1});
    drawPlane();

    tilingOn(true);
    flipUvs(true);
    bindTexture(gameState->textures["menu_items"]);
    scale({128, 64, 1});
    if (menuIndex == 0) {
        tintRed();
    } else {
        tintWhite();
    }
    tileData(0, 0, 128, 64);
    location({window_width/2, 450, -1});
    drawPlane();

    tint(glm::vec4{1, 1, 1, 1});
    tileData(1, 0, 128, 64);
    location({window_width/2, 380, -1});
    if (menuIndex == 1) {
        tintRed();
    } else {
        tintWhite();
    }
    drawPlane();

    tileData(2, 0, 128, 64);
    location({window_width/2, 310, -1});
    if (menuIndex == 2) {
        tintRed();
    } else {
        tintWhite();
    }
    drawPlane();

    // Cursor
    tileData(0, 1, 64, 64);
    tintRed();
    scale({64, 64, 1});
    location({window_width/2 - 100, 450 - (menuIndex * 70), -1});
    drawPlane();
    tintWhite();

    tilingOn(false);
    flipUvs(false);

}
