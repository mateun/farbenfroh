//
// Created by mgrus on 27.03.2025.
//

#include "EditorLauncher.h"

#include <engine/graphics/ui/ButtonWidget.h>
#include <engine/graphics/ui/LabelWidget.h>
#include <engine/graphics/ui/SplitterWidget.h>
#include <engine/graphics/ui/ToolBarWidget.h>
#include <engine/graphics/Texture.h>
#include <engine/graphics/ui/Menu.h>
#include <engine/graphics/ui/MenuBar.h>
#include <engine/graphics/ui/MenuItem.h>

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

    auto mainWidget = std::make_shared<Widget>();
    auto topToolbar = std::make_shared<ToolBarWidget>();
    topToolbar->setId("toolbar");
    auto areaLayout = std::make_shared<AreaLayout>(topToolbar, nullptr, nullptr, nullptr, mainSplitter);
    mainWidget->setLayout(areaLayout);
    mainWidget->addChild(topToolbar);
    mainWidget->addChild(mainSplitter);

    // Add buttons to the toolbar:
    auto btnStart = std::make_shared<ButtonWidget>();
    auto btnStop = std::make_shared<ButtonWidget>();
    auto btnPause = std::make_shared<ButtonWidget>();
    auto startButtonTexture = std::make_shared<Texture>("../assets/button_start_path.png");
    btnStart->setTexture(startButtonTexture);
    btnStop->setTexture(startButtonTexture);
    btnPause->setTexture(startButtonTexture);
    btnStart->setId("button_start");
    btnStop->setId("button_stop");
    btnPause->setId("button_pause");
    topToolbar->addChild(btnStart);
    topToolbar->addChild(btnStop);
    topToolbar->addChild(btnPause);

    auto menuFile = std::make_shared<Menu>("File");
    auto menuNew = std::make_shared<Menu>("New");
    auto menuOpen = std::make_shared<Menu>("Open");
    auto menuItemNewGame = std::make_shared<MenuItem>("Game");
    menuItemNewGame->setId("menu_item_new_game");
    menuNew->setId("file_new");
    menuNew->addSubMenu(menuItemNewGame);
    menuFile->addSubMenu(menuNew);
    menuFile->addSubMenu(menuOpen);

    auto menuGame = std::make_shared<Menu>("GameObjects");

    auto mainMenuBar = std::make_shared<MenuBar>();
    mainMenuBar->addMenu(menuFile);
    mainMenuBar->addMenu(menuGame);
    mainMenuBar->addMenu(std::make_shared<Menu>("About"));
    mainWidget->setMenuBar(mainMenuBar);

    setTopLevelWidget(mainWidget);
}

