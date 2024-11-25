//
// Created by mgrus on 13.01.2024.
//


#include "animation_editor.h"
#include "mars_attack.h"

extern GameState* gameState;
static Texture* jointNamesTexture  = nullptr;
static Texture* jointHeaderTexture  = nullptr;
static Texture* topMenuTextTexture  = nullptr;
static Texture* fileMenuTextTexture  = nullptr;
static AnimEditorState* animEditorState = nullptr;
static void renderBar(int w, int h, int x, int y, float depth = -0.6);

void initAnims()  {
    gameState->meshes["sk_plane"] = loadMeshFromFile("../assets/skeletal_plane.gltf");
    gameState->textures["skeleton_debug"] = createEmptyTexture(512, 512);
    jointHeaderTexture = createTextTexture(128, 24);
    jointNamesTexture = createTextTexture(512, 1024);
    topMenuTextTexture = createTextTexture(512, 16);
    fileMenuTextTexture = createTextTexture(192, 192);
    animEditorState = new AnimEditorState();
    animEditorState->menuState = MenuState::None;
}

/**
 * This checks if the mouse is over ANY
 * menu item inlcuding submenuitems.
 */
void checkMouseMainMenu() {
    animEditorState->prevMenuState = animEditorState->menuState;
    animEditorState->menuIsClicked = false;

    if (animEditorState->lastMenuClicked == MenuState::File) {
        if (mouse_x >= 18 && mouse_x < 54
            && mouse_y >= window_height - 55 && mouse_y < window_height - 35) {
            animEditorState->menuState = MenuState::Import;
            animEditorState->menuIsClicked = lbuttonUp;
        } else {
            animEditorState->menuState = MenuState::File;
        }
    }
    else if (mouse_x >= 1 && mouse_x < 34
        && mouse_y >= window_height - 30 && mouse_y < window_height - 5) {
        animEditorState->menuState = MenuState::File;
        animEditorState->menuIsClicked = lbuttonUp;
        if (lbuttonUp) {
            printf("registered file menu selection!\n");
        }

    }
    else if (mouse_x >= 35 && mouse_x < 80
             && mouse_y >= window_height - 30 && mouse_y < window_height - 5) {
        animEditorState->menuState = MenuState::Edit;
        animEditorState->menuIsClicked = lbuttonUp;
    }
    else {
        animEditorState->menuState = MenuState::None;
    }


    if (lbuttonUp == true) {
        if (animEditorState->menuIsClicked == true) {
            animEditorState->lastMenuClicked = animEditorState->menuState;
        } else {
            // We clicked but NOT inside any menu
            animEditorState->lastMenuClicked = MenuState::None;
        }
    }

    // Maybe move this to separate function
    // Handling the actual menu item clicked
    if (animEditorState->menuIsClicked) {
      if (animEditorState->lastMenuClicked == MenuState::Import) {
          printf("import clicked!\n");
          animEditorState->lastMenuClicked = MenuState::None;
      }
    }


}

void updateAnimEditor() {
    static bool first = true;
    if (first) {
        first = false;
        initAnims();
    }

    if (isKeyDown(VK_ESCAPE)) {
        gameState->level = Level::Gameplay;
    }

    checkMouseMainMenu();


}

void renderJointHeader() {
    foregroundColor({.1, .7, .5, 1});
    font(gameState->font);

    // First the static header, no scrolling here
    clearText(jointHeaderTexture);
    addText("Joints: ", jointHeaderTexture, 10, 10);
    addText("------- ", jointHeaderTexture, 8, 20);
    panUVS({0, 0});
    scale({128, 24, 1});
    location(glm::vec3{(float)  jointHeaderTexture->bitmap->width / 2 + 2, (float)window_height - 40, -.9});
    bindTexture(jointHeaderTexture);
    drawPlane();
}

void renderBar(int w, int h, int x, int y, float depth) {
    bindTexture(nullptr);
    scale({w, h, 1});
    location({x, y, depth});
    drawPlane();
}

/**
 * Allows for changing a text color for a given menu item.
 * @param state For which menuState (item) to check.
 */
void menuColor(MenuState state) {
    if (animEditorState->menuState == state) {
        foregroundColor({1, 0, 0, 1});
    } else {
        foregroundColor({.7, .7, .73, 1});
    }
}

void renderTopBar() {
    foregroundColor({0.1, 0.1, 0.1, 1});
    renderBar(window_width, 32, window_width/2, window_height-16);

    foregroundColor({.8, .8, .8, 1});
    font(gameState->font);

    // First the static header, no scrolling here
    clearText(topMenuTextTexture);
    menuColor(MenuState::File);
    addText("File", topMenuTextTexture, 5, 8);
    menuColor(MenuState::Edit);
    addText("Edit", topMenuTextTexture, 45, 8);
    panUVS({0, 0});
    scale({512, 16, 1});
    location(glm::vec3{(float)  topMenuTextTexture->bitmap->width / 2, (float)window_height - 16, -.59});
    bindTexture(topMenuTextTexture);
    drawPlane();
}

void renderFileMenu() {
    foregroundColor({0.15, 0.15, 0.15, 1});
    renderBar(256, 256, 128, window_height-128-32);
    foregroundColor({0.1, 0.1, 0.1, 1});
    renderBar(252, 253, 125 + 4, window_height-127-32, -0.5);
    foregroundColor({0.7, 0.7, 0.7, 1});
    clearText(fileMenuTextTexture);
    menuColor(MenuState::Import);
    addText("Import", fileMenuTextTexture, 16, 8);
    menuColor(MenuState::Export);
    addText("Export", fileMenuTextTexture, 16, 28);
    scale({192, 192, 1});
    location(glm::vec3{(float)  fileMenuTextTexture->bitmap->width / 2, (float)window_height - 140, -.49});
    bindTexture(fileMenuTextTexture);
    drawPlane();

}

void renderAnimEditorMainMenu() {
    renderTopBar();
    if (animEditorState->lastMenuClicked == MenuState::File || animEditorState->menuState == MenuState::Import || animEditorState->menuState == MenuState::Export) {
        renderFileMenu();
    }
}

std::string toString(MenuState menuState) {
    if (menuState == MenuState::File) {
        return "File";
    }else if (menuState == MenuState::Edit) {
        return "Edit";
    } else if (menuState == MenuState::Import) {
        return "Import";
    } else {
        return "undef";
    }
}

void renderAnimEditor() {

    renderAnimEditorMainMenu();

    auto skeleton = gameState->meshes["sk_plane"]->skeleton;
    auto tex = gameState->textures["skeleton_debug"];
    auto pixels = (uint32_t*)tex->bitmap->pixels;

    for (int i = 0; i < 512*512*4; i++) {
        tex->bitmap->pixels[i] = 90;
    }

    for (int i = 0; i< 512; i++) {
        int pixelOffset = i + 256 * 512;
        pixels[pixelOffset] = 0;
        pixels[pixelOffset] |= 15 << 24;
        pixels[pixelOffset] |= 15 << 16;
        pixels[pixelOffset] |= 15 << 8;
        pixels[pixelOffset] |= 255 << 0;

        pixelOffset = 256 + i * 512;
        pixels[pixelOffset] = 0;
        pixels[pixelOffset] |= 15 << 24;
        pixels[pixelOffset] |= 15 << 16;
        pixels[pixelOffset] |= 15 << 8;
        pixels[pixelOffset] |= 255 << 0;
    }

    int jointCount = 1;
    for (auto j : skeleton->joints) {

        glm::vec4 location = glm::vec4(j->translation, 1);
        location = {0, 0, 0, 1};
        glm::vec4 locTransformed = (j->globalTransform * location) * 30.0f;
        //printf("j: %s %f/%f\n", j->name.c_str(), locTransformed.x, locTransformed.y);
        glm::vec2 locationInBitmap = {256 + locTransformed.x , 256 + locTransformed.y};
        locationInBitmap = {(int)locationInBitmap.x, (int) locationInBitmap.y};

        for (int x = 0; x < 4; x++) {
            for (int y = 0; y < 4; y++) {
                int pixelOffset = ((-2 + locationInBitmap.x + x) + ((-2 + y+locationInBitmap.y) * 512));
                pixels[pixelOffset] = 0;
                pixels[pixelOffset] |= 255 << 24;
                if (jointCount == 1) {
                    pixels[pixelOffset] |= 50 << 16;
                    pixels[pixelOffset] |= 255 << 8;
                }
                if (jointCount == 2) {
                    pixels[pixelOffset] |= 220 << 16;
                    pixels[pixelOffset] |= 220 << 8;
                }
                pixels[pixelOffset] |= 255 << 0;
            }
        }
        jointCount++;
    }
    updateTexture(512, 512, tex);

    bindCamera(gameState->uiCamera);
    lightingOff();

    bindTexture(tex);

    location({window_width/2, window_height/2, -0.9});
    scale({512, 512, 1});
    drawPlane();
    scale({1, 1, 1});




    // Joint names rendering
    {
        renderJointHeader();

        // Scrolling for the joint, might be many:
        static float vertScroll = 0;
        if (isKeyDown(VK_DOWN)) {
            vertScroll -= 0.2 * ftSeconds;
        }
        if (isKeyDown(VK_UP)) {
            vertScroll += 0.2 * ftSeconds;
        }
        panUVS({0, vertScroll});

        clearText(jointNamesTexture);
        int jcount = 0;
        for (auto j : skeleton->joints) {
            addText(j->name, jointNamesTexture, 20, 30 + (jcount*20));
            jcount++;
        }

        scale({512, 1024, 1});
        location(glm::vec3{(float)  jointNamesTexture->bitmap->width / 2 + 2, (float)window_height/2 - 200, -.7});
        bindTexture(jointNamesTexture);
        drawPlane();
    }
    panUVS({0, 0});


    // FPS Text rendering
    {
        foregroundColor({.1, .7, .1, 1});
        char buf[190];
        sprintf_s(buf, 190, "FT:%6.1fmcs %4d/%4d %s %s",
                  ftMicros, mouse_x, mouse_y,
                  toString(animEditorState->lastMenuClicked).c_str(),
                  toString(animEditorState->menuState).c_str());
        font(gameState->font);
        updateAndDrawText(buf, gameState->textures["fps"], (window_width - gameState->textures["fps"]->bitmap->width / 2) + 140,
                          12);
    }



}
