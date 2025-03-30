//
// Created by mgrus on 27.03.2025.
//

#include "EditorLauncher.h"

#include <engine/graphics/ui/LabelWidget.h>
#include <engine/graphics/ui/SplitterWidget.h>

std::shared_ptr<Application> app;
std::shared_ptr<Application> getApplication() {
    if (!app) {
        app = std::make_shared<EditorLauncher>(800, 600, false);
    }
    return app;
}

EditorLauncher::EditorLauncher(int width, int height, bool fullscreen) : Application(width, height, fullscreen){
}

void EditorLauncher::onCreated() {

    auto vboxLayout = std::make_shared<VBoxLayout>();
    auto leftVBox = std::make_shared<Widget>();
    auto rightVBox = std::make_shared<Widget>();
    leftVBox->setLayout(vboxLayout);
    rightVBox->setLayout(vboxLayout);

    std::shared_ptr<TrueTypeFont> fontConsola = std::make_shared<TrueTypeFont>("../assets/consola.ttf", 16);
    auto lblProjects = std::make_shared<LabelWidget>("Projects", fontConsola);
    auto lblSettings = std::make_shared<LabelWidget>("Settings", fontConsola);
    auto lblProjects2 = std::make_shared<LabelWidget>("Actions", fontConsola);
    auto lblNewMesh = std::make_shared<LabelWidget>("NewMesh", fontConsola);
    leftVBox->addChild(lblProjects);
    leftVBox->addChild(lblSettings);
    rightVBox->addChild(lblProjects2);
    rightVBox->addChild(lblNewMesh);

    auto mainSplitter = std::make_shared<SplitterWidget>(SplitterType::Vertical, leftVBox, rightVBox);

    setTopLevelWidget(mainSplitter);
}

