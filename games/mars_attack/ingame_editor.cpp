//
// Created by mgrus on 15.01.2024.
//
#include "ingame_editor.h"

static void renderColoredRectangle(int w, int h, int x, int y, float depth);
static void renderPlayControlWindowBody(EditorWindow* win);
static InGameEditorState* inGameEditorState = nullptr;



/**
 * Checks if the mouse cursor is over a window in general.
 * @param window
 * @return
 */
bool checkHoverWindow(EditorWindow* window) {
    int hw = window->size.x / 2;
    int hh = window->size.y / 2;
    return (mouse_x >= window->location.x - hw && mouse_x <= window->location.x + hw &&
        mouse_y >= window->location.y - hh && mouse_y <= window->location.y + hh);
}

/**
 * Checks if the mouse cursor is over the window header.
 * @param window
 * @return
 */
bool checkHoverWindowHeader(EditorWindow* window) {
    int headerSize = window->headerHeight;
    int halfHeight = window->size.y / 2;
    int halfWidth = window->size.x / 2;
    return (mouse_x >= window->location.x -halfWidth && mouse_x <= window->location.x + halfWidth &&
            mouse_y >= window->location.y + halfHeight - headerSize && mouse_y <= window->location.y + halfHeight);
}

void mouseWindowHover() {
    bool hovered = false;
    for (auto window : inGameEditorState->allWindows) {
        hovered = checkHoverWindow(window);
        if (hovered) {
            inGameEditorState->hoveredWindow = window;

            if (!window->headless) {
                inGameEditorState->focusType = checkHoverWindowHeader(window) ? WindowFocusType::Header
                                                                              : WindowFocusType::Body;
                if (inGameEditorState->focusType == WindowFocusType::Header && !inGameEditorState->draggedWindow) {
                    auto offsetX = window->location.x - mouse_x;
                    auto offsetY = window->location.y - mouse_y;
                    inGameEditorState->mouseInHeader = {offsetX, offsetY};
                }
            }

            // Give the hovered window a chance to do any specfic logic involving the
            // mouse within its body.
            int winX = mouse_x - (window->location.x- (window->size.x / 2));
            int winY = mouse_y - (window->location.y -  (window->size.y / 2));
            window->winmouse = {winX, winY};
            if (window->mouseCheckBodyFunction) {
                window->mouseCheckBodyFunction(window);
            }
            break;
        }

    }

    // Clear currently hovered window if we did not find any hover
    if (!hovered) {
        inGameEditorState->hoveredWindow = nullptr;
    }

}

void mouseCheckPlayControlWindow(EditorWindow* win) {
    auto customData = (PlayIconBar*) win->customData;
    int selectionIndex = (win->winmouse.x + 24 - (win->size.x / 2)) / 16;
    printf("selIndex: %d\n", selectionIndex);
    if (selectionIndex == 0) {
        customData->hoverButton = IconButtonType::Play;
    } else if (selectionIndex == 1) {
        customData->hoverButton = IconButtonType::Pause;
    } else if (selectionIndex == 2) {
        customData->hoverButton = IconButtonType::Stop;
    } else {
        customData->hoverButton = IconButtonType::None;
    }
}

void mouseCheckObjectWindow(EditorWindow* win) {
    auto customData = (ObjectWindowData*) win->customData;
    customData->selectionIndex = (win->size.y - (win->winmouse.y + win->headerHeight)) / 16;
}

void renderObjectWindowBody(EditorWindow* objWin) {
    auto objectData = (ObjectWindowData*) objWin->customData;
    auto tex = objWin->frameBufferTexture;
    auto pixels = (uint32_t*) tex->bitmap->pixels;
    int xPos = 4;
    int yPos = objWin->size.y - objWin->headerHeight - 10;

    // Render background rect highlighting the selected
    if (objectData->selectionIndex >= 0 && objectData->selectionIndex < gameState->allGameObjects.size()) {
        xPos = 12;
        yPos = (objWin->size.y - objWin->headerHeight - 16) - (objectData->selectionIndex * 16);
        for (int x = 0; x < 200;x++) {
            for(int y = 0; y < 16; y++) {
                int offset = (xPos +x)  + ((-2+ y + yPos) * objWin->size.x);
                pixels[offset] = 52 << 0;      // R
                pixels[offset] |= 52 << 8;      // G
                pixels[offset] |= 52 << 16;     // B
                pixels[offset] |= 255 << 24;    // A
            }
        }
    }

    xPos = 4;
    yPos = objWin->size.y - objWin->headerHeight - 10;
    for (auto go : gameState->allGameObjects) {
        // Render small dash in front of the name
        for (int x = 0; x < 8;x++) {
            for(int y = 0; y < 1; y++) {
                int offset = (xPos +x)  + ((y + yPos) * objWin->size.x);
                pixels[offset] = 180 << 0;      // R
                pixels[offset] |= 180 << 8;      // G
                pixels[offset] |= 25 << 16;     // B
                pixels[offset] |= 255 << 24;    // A
            }
        }

        bitmapBlitModeOn();
        addText(go->name, tex, xPos + 10, objWin->size.y - yPos + 3);
        yPos -= 16;
    }


    // Debug output
    addText(std::to_string(objWin->winmouse.x) + "/" + std::to_string(objWin->winmouse.y), tex, 100, 16);
    addText("selIndex:" + std::to_string(objectData->selectionIndex), tex, 100, 245);
    //bitmapBlitModeOff();



}

void mouseClickPlayControlWindow(EditorWindow* win) {
    auto data = (PlayIconBar*) win->customData;
    if (data->hoverButton == IconButtonType::Pause) {
        if (data->pressedButton != IconButtonType::Pause) {
            data->pressedButton = IconButtonType::Pause;
            gameState->paused = true;
        } else {
            gameState->paused = false;
            data->pressedButton = IconButtonType::Play;
        }
    }


}

void renderScriptingWindowBody(EditorWindow* win) {

}

/**
 * This should be called after the hovering has been checked.
 * We will assign the focused window to be the one which was last hovered over.
 */
void mouseWindowClickCheck() {

    // First, check dragging
    if (lbuttonUp) {
        if (inGameEditorState->draggedWindow ) {
            inGameEditorState->draggedWindow = nullptr;
        }
        else if (inGameEditorState->hoveredWindow && inGameEditorState->focusType == WindowFocusType::Header) {
            inGameEditorState->draggedWindow = inGameEditorState->hoveredWindow;
            return;
        }

        // If we are here, we are not dragging, so we can pass the information to the hovered window:
        if (inGameEditorState->hoveredWindow) {
            if (inGameEditorState->hoveredWindow->mouseClickFunction) {
                inGameEditorState->hoveredWindow->mouseClickFunction(inGameEditorState->hoveredWindow);
            }

        }
    }

}

/**
 * If we have a currently dragged window we move it along the current mouse position.
 */
void windowDrag() {
    if (inGameEditorState->draggedWindow) {
        auto win = inGameEditorState->draggedWindow;
        inGameEditorState->draggedWindow->location = { mouse_x+ inGameEditorState->mouseInHeader.x, mouse_y + inGameEditorState->mouseInHeader.y};
    }
}

static void initInGameEditor() {
    inGameEditorState = new InGameEditorState();
    inGameEditorState->objectWindow = new EditorWindow();
    auto objWin = inGameEditorState->objectWindow;
    objWin->title = "Game objects";
    objWin->customData = new ObjectWindowData();
    objWin->size = {256, 256};
    objWin->location.x = objWin->size.x/2 + 5;
    objWin->location.y = window_height - objWin->size.y/2 - 10;
    objWin->frameBufferTexture = createEmptyTexture(objWin->size.x, objWin->size.y);
    objWin->renderBodyFunction = renderObjectWindowBody;
    objWin->mouseCheckBodyFunction = mouseCheckObjectWindow;
    inGameEditorState->allWindows.push_back(objWin);

    inGameEditorState->playControlsWindow = new EditorWindow();
    inGameEditorState->playControlsWindow->headless = true;
    inGameEditorState->playControlsWindow->customData = new PlayIconBar();
    inGameEditorState->playControlsWindow->renderBodyFunction = renderPlayControlWindowBody;
    inGameEditorState->playControlsWindow->mouseCheckBodyFunction = mouseCheckPlayControlWindow;
    inGameEditorState->playControlsWindow->mouseClickFunction = mouseClickPlayControlWindow;
    inGameEditorState->playControlsWindow->size = {256, 32};
    inGameEditorState->playControlsWindow->location = glm::vec2{window_width/2, window_height - 12};
    inGameEditorState->playControlsWindow->frameBufferTexture =  createEmptyTexture(256, 32);
    inGameEditorState->allWindows.push_back(inGameEditorState->playControlsWindow);

    inGameEditorState->scripingWindow = new EditorWindow();
    inGameEditorState->scripingWindow->title = "Scripting";
    inGameEditorState->scripingWindow->size = {512, 512};
    inGameEditorState->scripingWindow->location = {window_width/2, window_height/2};
    inGameEditorState->scripingWindow->frameBufferTexture = createEmptyTexture(512, 512);
    inGameEditorState->allWindows.push_back(inGameEditorState->scripingWindow);

    loadBitmap("../assets/editor_icons.png", &inGameEditorState->editorIconBitmap);
    gameState->paused = true;

}

void updateInGameEditor() {
    if (gameState->level != Level::InGameEditor) {
        return;
    }

    if (!inGameEditorState) {
        initInGameEditor();
    }

    if (keyPressed(VK_ESCAPE)) {
        gameState->level = Level::Gameplay;
    }

    mouseWindowHover();
    mouseWindowClickCheck();
    windowDrag();
}

// Deprecated
void renderObjectWindow() {
    auto tex = inGameEditorState->objectWindow->frameBufferTexture;
    auto pixels = (uint32_t*)(tex->bitmap->pixels);

    // Clear the window
    for (int i = 0; i < 256*256; i++) {
        pixels[i] = 0;
        // Full alpha in last byte
        pixels[i] |= 215 << 24;
    }

    // Header background
    for (int i = 0; i< 256; i++) {
        for (int y = 0; y < 24; y++) {
            int pixelOffset = i + ((255-y) * 256);
            pixels[pixelOffset] = 0;
            if (inGameEditorState->hoveredWindow == inGameEditorState->objectWindow) {
                if (inGameEditorState->focusType == WindowFocusType::Header) {
                    pixels[pixelOffset] |= 225 << 0;     // R
                } else {
                    pixels[pixelOffset] |= 100 << 0;     // R
                }

            } else {
                pixels[pixelOffset] |= 25 << 0;     // R
            }
            pixels[pixelOffset] |= 15 << 8;     // G
            pixels[pixelOffset] |= 25 << 16;   // B
            pixels[pixelOffset] |= 215 << 24;   // A
        }

    }

    // Header text
    bitmapBlitModeOn();
    foregroundColor({0.8, .8, .8, 1});
    addText("Objects", tex, 10, 14);
    bitmapBlitModeOff();
    updateTexture(256, 256, tex);

    bindTexture(tex);
    location({inGameEditorState->objectWindow->location.x, inGameEditorState->objectWindow->location.y, -0.2});
    scale({256, 256, 1});
    drawPlane();

}

void foregroundForIcon(PlayIconBar* data, IconButtonType type) {
    if (data->hoverButton == type) {
        foregroundColor({0, 0.7, 0, 1});
    } else {
        foregroundColor({0.8, 0.8, 0.8, 1});
    }
    if (data->pressedButton == type) {
        foregroundColor({0, 0.7, 0.7, 1});
    }

}

void renderPlayControlWindowBody(EditorWindow* win) {
    auto tex = win->frameBufferTexture;
    auto data = (PlayIconBar*) win->customData;

    int posX = win->size.x/2 - (3*16/2);
    glm::vec2 locationInBitmap = {posX, 5};
    renderTargetBitmap(tex->bitmap);
    bitmapBlitModeOn();
    foregroundForIcon(data, IconButtonType::Play);
    drawBitmapTile(locationInBitmap.x, locationInBitmap.y, 16, 0, 15, inGameEditorState->editorIconBitmap);
    foregroundForIcon(data, IconButtonType::Pause);
    drawBitmapTile(locationInBitmap.x + 16, locationInBitmap.y, 16, 1, 15, inGameEditorState->editorIconBitmap);
    foregroundForIcon(data, IconButtonType::Stop);
    drawBitmapTile(locationInBitmap.x + 32, locationInBitmap.y, 16, 2, 15, inGameEditorState->editorIconBitmap);
    bitmapBlitModeOff();
    updateTexture(win->size.x, win->size.y, tex);
    bindTexture(tex);
    location({win->location, -0.19});
    scale({win->size, 1});
    drawPlane();
}

// Deprecated
void renderTopBarIcons() {
    auto win = inGameEditorState->playControlsWindow;
    auto tex = inGameEditorState->playControlsWindow->frameBufferTexture;
    auto pixels = (uint32_t*) tex->bitmap->pixels;

    // Clear the window
    for (int i = 0; i < 256*32; i++) {
        pixels[i] = 0;
        // Full alpha in last byte
        pixels[i] |= 255 << 24;
    }

    glm::vec2 locationInBitmap = {128 - (3*20/2), 5};
    renderTargetBitmap(tex->bitmap);
    bitmapBlitModeOn();
    foregroundColor({0, 1, 0, 1});
    drawBitmapTile(locationInBitmap.x, locationInBitmap.y, 16, 0, 15, inGameEditorState->editorIconBitmap);
    foregroundColor({.5, .5, .5, 1});
    drawBitmapTile(locationInBitmap.x + 20, locationInBitmap.y, 16, 1, 15, inGameEditorState->editorIconBitmap);
    drawBitmapTile(locationInBitmap.x + 40, locationInBitmap.y, 16, 2, 15, inGameEditorState->editorIconBitmap);
    bitmapBlitModeOff();
    updateTexture(win->size.x, win->size.y, tex);
    bindTexture(tex);
    location({win->location, -0.19});
    scale({win->size, 1});
    drawPlane();
}

void renderWindowHeader(EditorWindow* win) {
    auto tex = win->frameBufferTexture;
    auto pixels = (uint32_t*)(tex->bitmap->pixels);

    for (int i = 0; i< win->size.x; i++) {
        for (int y = 0; y < win->headerHeight; y++) {
            int pixelOffset = i + ((win->size.y-1-y) * win->size.x);
            pixels[pixelOffset] = 0;
            if (inGameEditorState->hoveredWindow == win) {
                if (inGameEditorState->focusType == WindowFocusType::Header) {
                    pixels[pixelOffset] |= 225 << 0;     // R
                } else {
                    pixels[pixelOffset] |= 100 << 0;     // R
                }

            } else {
                pixels[pixelOffset] |= 25 << 0;     // R
            }
            pixels[pixelOffset] |= 15 << 8;     // G
            pixels[pixelOffset] |= 25 << 16;   // B
            pixels[pixelOffset] |= 215 << 24;   // A
        }

    }

    // Header text
    bitmapBlitModeOn();
    foregroundColor({0.8, .8, .8, 1});
    addText(win->title, tex, 10, 14);
    bitmapBlitModeOff();

}

static void ShowGameObject(GameObject* go, int uid)
{
    // Use object uid as identifier. Most commonly you could also use the object pointer as a base ID.
    ImGui::PushID(uid);

    // Text and Tree nodes are less high than framed widgets, using AlignTextToFramePadding() we add vertical spacing to make the tree lines equal high.
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    bool node_open = ImGui::TreeNode("GameObject", "%s_%u", go->name.c_str(), uid);
    ImGui::TableSetColumnIndex(1);
    ImGui::Text("Data");

    if (node_open) {
        ImGui::PushID(0); // Use field index as identifier.
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;
        ImGui::TreeNodeEx("##pos", flags, "Location", 0);
        // Get location based on game object type
        auto location = go->location;
        if (go->type == GameObjectType::Enemy) {
            location = ((Enemy*) go->object)->location;
        } else if (go->type == GameObjectType::Projectile) {
            location = ((Projectile*) go->object)->location;
        }
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::Text("%f", location.x);
        ImGui::TableSetColumnIndex(2);
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::Text("%f", location.y);
        ImGui::TableSetColumnIndex(3);
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::Text("%f", location.z);
            //ImGui::DragFloat("##value", &placeholder_members[i], 0.01f);
            //ImGui::NextColumn();
        ImGui::PopID();
        ImGui::TreePop();
    }

    if (node_open)
    {
//        static float placeholder_members[8] = { 0.0f, 0.0f, 1.0f, 3.1416f, 100.0f, 999.0f };
//        for (int i = 0; i < 8; i++)
//        {
//            ImGui::PushID(i); // Use field index as identifier.
//            if (i < 2)
//            {
//                //ShowGameObject("Child");
//            }
//            else
//            {
//                // Here we use a TreeNode to highlight on hover (we could use e.g. Selectable as well)
//                ImGui::TableNextRow();
//                ImGui::TableSetColumnIndex(0);
//                ImGui::AlignTextToFramePadding();
//                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;
//
//                ImGui::TreeNodeEx("Position", flags, "Field_%d", i);
//
//                ImGui::TableSetColumnIndex(1);
//                ImGui::SetNextItemWidth(-FLT_MIN);
//
//                if (i >= 5)
//                    ImGui::InputFloat("##value", &placeholder_members[i], 1.0f);
//                else
//                    ImGui::DragFloat("##value", &placeholder_members[i], 0.01f);
//                ImGui::NextColumn();
//            }
//            ImGui::PopID();
//        }
//        ImGui::TreePop();
    }
    ImGui::PopID();
}

void renderWindowsImGui() {

    ImGui::SetNextWindowSize(ImVec2(430, 450), ImGuiCond_FirstUseEver);
    bool p_open = true;
    if (!ImGui::Begin("Tankofant InGame Editor", &p_open))
    {
        ImGui::End();
        return;
    }
    if (ImGui::Button("Play")) {
        gameState->paused = false;
    }

    if (ImGui::Button("Pause")) {
        gameState->paused = true;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
    if (ImGui::BeginTable("GameObjects", 4, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY))
    {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("GameObject");
        ImGui::TableSetupColumn("Data");
        ImGui::TableHeadersRow();

        // Iterate placeholder objects (all the same data)

        int index = 0;
        for (auto go : gameState->allGameObjects) {
            ShowGameObject(go, index++);

        }

        ImGui::EndTable();
    }
    ImGui::PopStyleVar();
    ImGui::End();
}

void renderWindows() {
    for (auto win : inGameEditorState->allWindows) {
        if (!win->headless) {
            renderWindowHeader(win);
        }

        // TODO actually fill the window body contents here!
        if (win->renderBodyFunction) {
            win->renderBodyFunction(win);
        }


        updateTexture(win->size.x, win->size.y, win->frameBufferTexture);

        bindTexture(win->frameBufferTexture);
        location({win->location, -0.2});
        scale({win->size, 1});
        drawPlane();

    }
}

void clearWindows() {
    for (auto win: inGameEditorState->allWindows) {
        auto tex = win->frameBufferTexture;
        auto pixels = (uint32_t*)(tex->bitmap->pixels);

        // Clear the window to black with alpha set to 255.
        for (int i = 0; i < win->size.x*win->size.y; i++) {
            pixels[i] = 0;
            pixels[i] |= 215 << 24;
        }
    }

}

/**
 * This renders an overlay UI to allow for editing the running game.
 * The game can be paused, game objects be inspected, a debug flycam
 * can be activated and new game objects can be instantiated.
 *
 */
#define USE_IMGUI
void renderInGameEditor() {

    if (gameState->level != Level::InGameEditor) {
        return;
    }


#ifndef USE_IMGUI

    bindCamera(gameState->uiCamera);
    lightingOff();

//    foregroundColor({0.2, 0.2, 0.2, 1});
//    renderColoredRectangle(window_width, 32, window_width/2, window_height - 16, -0.69);
   // renderTopBarIcons();

    clearWindows();
    renderWindows();
    //renderObjectWindow();
#else
    renderWindowsImGui();

#endif

}

void renderColoredRectangle(int w, int h, int x, int y, float depth) {
    bindTexture(nullptr);
    scale({w, h, 1});
    location({x, y, depth});
    drawPlane();
}
