//
// Created by mgrus on 15.01.2024.
//


#ifndef SIMPLE_KING_INGAME_EDITOR_H
#define SIMPLE_KING_INGAME_EDITOR_H

#include <functional>



struct ObjectWindowData {
    int selectionIndex = -1;
};

struct EditorWindow {
    std::string title = "-- window --";
    Texture* frameBufferTexture = nullptr;
    glm::vec2 location = {0, 0};
    glm::vec2 size = {256, 256};
    float headerHeight = 24;
    bool headless = false;
    std::function<void(EditorWindow*)> renderBodyFunction;
    std::function<void(EditorWindow*)> mouseCheckBodyFunction;
    std::function<void(EditorWindow*)> mouseClickFunction;
    glm::vec2 winmouse;
    void* customData = nullptr;
};

enum class WindowFocusType {
    Header,
    Body
};

enum class IconButtonType {
    None,
    Play,
    Pause,
    Stop
};

enum class ButtonState {
    None,
    Hover,
    Pressed
};

struct IconButton {
    IconButtonType type;
    glm::vec2 location;
    glm::vec2 size = {16, 16};

};

struct PlayIconBar {
    glm::vec2 location;
    IconButtonType hoverButton = IconButtonType::None;
    IconButtonType pressedButton = IconButtonType::Play;

};



struct InGameEditorState {
    EditorWindow* objectWindow = nullptr;
    EditorWindow* playControlsWindow = nullptr;
    EditorWindow* scripingWindow = nullptr;

    // This is the window which is currently dragged by its
    // header.
    EditorWindow* draggedWindow = nullptr;

    // Which window has the user last hovered?
    EditorWindow* hoveredWindow = nullptr;

    // Which window is currently in focus?
    // The user has last clicked on.
    EditorWindow* focusedWindow = nullptr;
    WindowFocusType focusType = WindowFocusType::Body;

    std::vector<EditorWindow*> allWindows;

    // These are coordinates relative to the window header
    // (in header space).
    glm::vec2 mouseInHeader = {0, 0};
    Bitmap* editorIconBitmap = nullptr;


};

void updateInGameEditor();
void renderInGameEditor();

#endif //SIMPLE_KING_INGAME_EDITOR_H
