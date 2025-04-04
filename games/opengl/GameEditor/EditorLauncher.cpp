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
#include <engine/graphics/ui/Spacer.h>
#include <engine/graphics/ui/FloatingWindow.h>
#include <engine/graphics/ui/MessageTransformer.h>

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
    rightVBox->setId("right_vbox");
    rightVBox->setBgColor({0.5, 0, 0.5, 1});
    leftVBox->setId("left_vbox");
    leftVBox->setBgColor({0.5, 0.5, 0.0, 1});
    leftVBox->setLayout(vboxLayout);
    rightVBox->setLayout(vboxLayout);

    std::shared_ptr<TrueTypeFont> fontConsola = std::make_shared<TrueTypeFont>("../assets/consola.ttf", 16);
    auto lblProjects = std::make_shared<LabelWidget>("Projects", fontConsola);
    auto lblSettings = std::make_shared<LabelWidget>("Settings", fontConsola);
    lblMouseCoords = std::make_shared<LabelWidget>("FrameCount", fontConsola);
    auto lblProjects2 = std::make_shared<LabelWidget>("Actions", fontConsola);
    auto lblNewMesh = std::make_shared<LabelWidget>("NewMesh", fontConsola);
    leftVBox->addChild(lblProjects);
    leftVBox->addChild(lblSettings);
    leftVBox->addChild(lblMouseCoords);
    rightVBox->addChild(lblProjects2);
    rightVBox->addChild(lblNewMesh);

    auto mainSplitter = std::make_shared<SplitterWidget>(SplitterType::Vertical, leftVBox, rightVBox);
    mainSplitter->setId("main_splitter");

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
    auto spacer = std::make_shared<Spacer>(glm::vec2{400, 0});
    topToolbar->addChild(spacer);
    topToolbar->addChild(btnStop);
    topToolbar->addChild(btnPause);

    auto menuFile = std::make_shared<Menu>("File");
    menuFile->setId("menu_file");
    auto menuNew = std::make_shared<Menu>("New");
    auto menuOpen = std::make_shared<Menu>("Open");
    menuOpen->setId("menu_open");
    auto menuItemNewGame = std::make_shared<MenuItem>("Game");
    menuItemNewGame->setId("menu_item_new_game");
    menuNew->setId("file_new");
    menuNew->addSubMenu(menuItemNewGame);
    menuFile->addSubMenu(menuNew);
    menuFile->addSubMenu(menuOpen);

    auto menuGameObjects = std::make_shared<Menu>("GameObjects");
    menuGameObjects->setId("menu_game_objects");
    auto menuNewCube = std::make_shared<MenuItem>("New Cube");
    menuNewCube->setId("menu_item_new_cube");
    menuGameObjects->addSubMenu(menuNewCube);
    auto menuNewSphere = std::make_shared<MenuItem>("NewSphere");
    menuGameObjects->addSubMenu(menuNewSphere);

    auto mainMenuBar = std::make_shared<MenuBar>();
    mainMenuBar->addMenu(menuFile);
    mainMenuBar->addMenu(menuGameObjects);
    auto menuAbout = std::make_shared<Menu>("About");
    menuAbout->setId("menu_about");
    mainMenuBar->addMenu(menuAbout);
    mainWidget->setMenuBar(mainMenuBar);

    auto consoleWidget = std::make_shared<Widget>();
    auto previewWidget = std::make_shared<Widget>();
    auto hSplitter = std::make_shared<SplitterWidget>(SplitterType::Horizontal, previewWidget, consoleWidget);
    hSplitter->setLayoutHint(LayoutHint{true, true});
    hSplitter->setId("h_splitter");
    consoleWidget->setId("console");
    previewWidget->setId("preview");
    rightVBox->addChild(hSplitter);

    mainWidget->setId("main_widget");
    setTopLevelWidget(mainWidget);

    auto floatingWindow1 = std::make_shared<FloatingWindow>();
    floatingWindow1->setId("floating_window1");
    floatingWindow1->setOrigin({200, 200});
    floatingWindow1->setSize({200, 150});
    addFloatingWindow(floatingWindow1);

    addMessageSubscriber(shared_from_this());
}

void EditorLauncher::doFrame() {
    // noop
}

void EditorLauncher::onFrameMessages(const std::vector<RawWin32Message> &msgs) {
    for (auto rm : msgs) {
        auto transformedMessage = MessageTransformer::transform(rm);
        if (transformedMessage.type == MessageType::MouseMove) {
            lblMouseCoords->setText("mouse: " + std::to_string(transformedMessage.mouseMoveMessage.x) +  "/" + std::to_string(transformedMessage.mouseMoveMessage.y));
        }
    }


}

