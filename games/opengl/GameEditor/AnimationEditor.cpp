//
// Created by mgrus on 23.03.2025.
//

#include "AnimationEditor.h"
#include <engine/graphics/Widget.h>

std::shared_ptr<Application> getApplication() {
    return std::make_shared<AnimationEditor>(800, 600, false);
}


AnimationEditor::AnimationEditor(int width, int height, bool fullscreen) : Application(width, height, fullscreen) {
}

/**
 * Here we create our actual editor widgets
 */
void AnimationEditor::onCreated() {
    setTopLevelWidget(std::make_shared<EmptyContainer>());

}
