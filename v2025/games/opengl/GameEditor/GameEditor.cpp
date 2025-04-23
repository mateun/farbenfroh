//
// Created by mgrus on 23.03.2025.
//

#include "GameEditor.h"
#include <engine/graphics/Widget.h>

#ifdef GAME_EDITOR
std::shared_ptr<Application> getApplication() {
    return std::make_shared<GameEditor>(800, 600, false);
}
#endif


GameEditor::GameEditor(int width, int height, bool fullscreen) : Application(width, height, fullscreen) {
}

/**
 * Here we create our actual editor widgets
 */
void GameEditor::onCreated() {
    setTopLevelWidget(std::make_shared<Widget>());

}
