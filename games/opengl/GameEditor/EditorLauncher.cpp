//
// Created by mgrus on 27.03.2025.
//

#include "EditorLauncher.h"

#include <engine/graphics/ui/LabelWidget.h>

std::shared_ptr<Application> getApplication() {
    return std::make_shared<EditorLauncher>(800, 600, false);
}

EditorLauncher::EditorLauncher(int width, int height, bool fullscreen) : Application(width, height, fullscreen){
}

void EditorLauncher::onCreated() {
    auto lblProjects = std::make_shared<LabelWidget>("Projects");
    setTopLevelWidget(lblProjects);
}
