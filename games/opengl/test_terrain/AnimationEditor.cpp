//
// Created by mgrus on 23.03.2025.
//

#include "AnimationEditor.h"

std::unique_ptr<Application> getApplication() {
    return std::make_unique<AnimationEditor>(800, 600, false);
}


AnimationEditor::AnimationEditor(int width, int height, bool fullscreen) : Application(width, height, fullscreen) {
}
