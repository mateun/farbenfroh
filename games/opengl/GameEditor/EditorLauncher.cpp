//
// Created by mgrus on 27.03.2025.
//

#include "EditorLauncher.h"

#include <iomanip>
#include <iostream>
#include <engine/algo/Bvh.h>
#include <engine/game/Timing.h>
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

#include <engine/graphics/ui/UIMessage.h>
#include <engine/graphics/ui/TextInput.h>
#include <engine/graphics/ui/TreeViewer.h>

std::shared_ptr<Application> app;
std::shared_ptr<Application> getApplication() {
    if (!app) {
        app = std::make_shared<EditorLauncher>(800, 600, false);
    }
    return app;
}

EditorLauncher::EditorLauncher(int width, int height, bool fullscreen) : Application(width, height, fullscreen){
}

void EditorLauncher::makeLegacyUI() {
     auto vboxLayout = std::make_shared<VBoxLayout>();
    vboxLayout->setMarginVertical(16);
    vboxLayout->setMarginHorizontal(16);
    auto leftVBox = std::make_shared<Widget>();
    auto rightVBox = std::make_shared<Widget>();
    rightVBox->setId("right_vbox");
    leftVBox->setId("left_vbox");
    auto layoutLeft = std::make_shared<VBoxLayout>();
    layoutLeft->setMarginHorizontal(0);
    layoutLeft->setMarginVertical(16);
    leftVBox->setLayout(layoutLeft);
    rightVBox->setLayout(vboxLayout);

    std::shared_ptr<TrueTypeFont> fontConsola = std::make_shared<TrueTypeFont>("../assets/consola.ttf", 16);
    auto lblProjects = std::make_shared<LabelWidget>("Projects", fontConsola);
    lblProjects->setId("lbl_projects");
    auto lblSettings = std::make_shared<LabelWidget>("Settings", fontConsola);
    lblMouseCoords = std::make_shared<LabelWidget>("Mouse", fontConsola);
    lblFrameTime = std::make_shared<LabelWidget>("FT", fontConsola);
    auto lblProjects2 = std::make_shared<LabelWidget>("Actions", fontConsola);
    auto lblNewMesh = std::make_shared<LabelWidget>("NewMesh", fontConsola);
    // leftVBox->addChild(lblProjects);
    // leftVBox->addChild(lblSettings);
    // leftVBox->addChild(lblMouseCoords);

    rightVBox->addChild(lblProjects2);
    rightVBox->addChild(lblNewMesh);

    auto levelViewPort = std::make_shared<Widget>();
    levelViewPort->setId("level_view_port");
    levelViewPort->setBgColor({1, 0,1,0.2});

    auto consoleWidget = std::make_shared<Widget>();
    consoleWidget->addChild(lblMouseCoords);
    auto gameObjectExplorer = std::make_shared<Widget>();
    auto hSplitter = std::make_shared<SplitterWidget>(SplitterType::Horizontal, levelViewPort, consoleWidget);
    hSplitter->setLayoutHint(LayoutHint{true, true});
    hSplitter->setId("h_splitter");
    consoleWidget->setId("console");

    gameObjectExplorer->setId("preview");
    auto rootGameObject = std::make_shared<TreeNode>(nullptr);
    rootGameObject->setText("Root Game Object");
    auto child1 = std::make_shared<TreeNode>(nullptr);
    auto child2 = std::make_shared<TreeNode>(nullptr);
    child1->setText("Child1");
    child2->setText("Child2");
    rootGameObject->addChild(child1);
    rootGameObject->addChild(child2);
    auto treeModel = std::make_shared<TreeModel>(rootGameObject);
    auto gameObjectTree = std::make_shared<TreeViewer>(treeModel);
    gameObjectExplorer->addChild(gameObjectTree);
    gameObjectExplorer->setLayout(std::make_shared<AreaLayout>(nullptr, nullptr, nullptr, nullptr, gameObjectTree));

    auto telemetryWidget = std::make_shared<Widget>();
    telemetryWidget->setId("telemetry");
    telemetryWidget->setLayout(std::make_shared<VBoxLayout>());
    telemetryWidget->setLayoutHint(LayoutHint{true, true});
    telemetryWidget->addChild(lblMouseCoords);
    telemetryWidget->addChild(lblFrameTime);

    auto leftSplitter = std::make_shared<SplitterWidget>(SplitterType::Horizontal, gameObjectExplorer , telemetryWidget);
    leftSplitter->setLayoutHint(LayoutHint{true, true});
    leftSplitter->setId("left_splitter");
    leftVBox->addChild(leftSplitter);

    rightVBox->addChild(hSplitter);

    auto consoleMain = std::make_shared<Widget>();
    consoleMain->setId("console_main");
    //consoleMain->setBgGradient({.11, 0.011, 0.011, 1});
    consoleWidget->setLayout(std::make_shared<AreaLayout>(nullptr, nullptr, nullptr,nullptr, consoleMain));
    consoleWidget->addChild(consoleMain);

    auto mainSplitter = std::make_shared<SplitterWidget>(SplitterType::Vertical, leftVBox, hSplitter);
    mainSplitter->setId("main_splitter");

    auto mainWidget = std::make_shared<Widget>();
    mainWidget->setBgGradient({0.01, 0.013, 0.0, 1}, {0.001, 0.0014, 0.0, 1});

    auto topToolbar = std::make_shared<ToolBarWidget>();
    topToolbar->setId("toolbar");
    topToolbar->setBgColor({0.02, .02, .02, 1});
    auto areaLayout = std::make_shared<AreaLayout>(topToolbar, nullptr, leftVBox, nullptr, mainSplitter);
    mainWidget->setLayout(areaLayout);
    mainWidget->addChild(topToolbar);
    mainWidget->addChild(mainSplitter);

    // Add buttons to the toolbar:
    auto startButtonTexture = std::make_shared<Texture>("../assets/button_start_path.png");
    auto btnStart = std::make_shared<ButtonWidget>(startButtonTexture);
    auto btnStop = std::make_shared<ButtonWidget>(startButtonTexture);
    auto btnPause = std::make_shared<ButtonWidget>(startButtonTexture);
    btnStart->setTexture(startButtonTexture);
    btnStart->addActionCallback([this](std::shared_ptr<Widget> action) {
        std::cout << "Start launching" << std::endl;
        auto floatingWindow = std::make_shared<FloatingWindow>();
        floatingWindow->setId("floating_window1");
        floatingWindow->setBgColor({0.02, .02, .02, 0.98});
        floatingWindow->setOrigin({200, 100});
        floatingWindow->setSize({8 * 40, 8 * 30});
        auto vbl = std::make_shared<VBoxLayout>();
        vbl->setMarginVertical(0);
        vbl->setMarginHorizontal(24);
        floatingWindow->setLayout(vbl);
        auto labelGameName = std::make_shared<LabelWidget>("Project Name", getDefaultMenuFont());
        labelGameName->setId("label_game_name");

        auto btnSaveGameProject = std::make_shared<ButtonWidget>("Save", getDefaultMenuFont());
        btnSaveGameProject->setId("btn_save_project");
        btnSaveGameProject->disable();  // Shall only enable when text is set
        btnSaveGameProject->addActionCallback([floatingWindow](std::shared_ptr<Widget> action) {
            // TODO add code for actually saving the game project
            floatingWindow->close();
        });

        auto txtProjectFolder = std::make_shared<TextInput>("Select directory", getDefaultMenuFont());
        txtProjectFolder->setId("txt_project_folder");
        txtProjectFolder->setBgColor({0.02, .02, .02, 1});


        auto txtGameName = std::make_shared<TextInput>("Enter project name", getDefaultMenuFont());
        txtGameName->addTextChangeListener([btnSaveGameProject, txtProjectFolder] (std::shared_ptr<TextInput> textInput) {
             if (!textInput->getText().empty() && !txtProjectFolder->getText().empty()) {
                 btnSaveGameProject->enable();
             }
             else {
                 btnSaveGameProject->disable();
             }
        });
        txtGameName->setId("txt_game_name");
        txtGameName->setTextColor({0.02, 0.02, 0.02, 1});

        txtProjectFolder->addTextChangeListener([btnSaveGameProject, txtGameName] (std::shared_ptr<TextInput> textInput) {
            if (!textInput->getText().empty() && !txtGameName->getText().empty()) {
               btnSaveGameProject->enable();
           }
           else {
               btnSaveGameProject->disable();
           }
        });

        auto spacer = std::make_shared<Spacer>(glm::vec2{0.0f, 20.0f});
        spacer->setBgColor({0.1, .1, .1, 1});

        floatingWindow->addChild(spacer);
        floatingWindow->addChild(labelGameName);
        floatingWindow->addChild(txtGameName);
        floatingWindow->addChild(std::make_shared<LabelWidget>("Project Folder", getDefaultMenuFont()));
        floatingWindow->addChild(txtProjectFolder);
        floatingWindow->addChild(spacer);
        floatingWindow->addChild(btnSaveGameProject);
        addFloatingWindow(floatingWindow);
    });
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

    mainWidget->setId("main_widget");
    setTopLevelWidget(mainWidget);

    addMessageSubscriber(shared_from_this());

}

void EditorLauncher::onCreated() {

    auto mainWidget = std::make_shared<Widget>();
    mainWidget->setId("main_widget");
    auto center = std::make_shared<Widget>();
    center->setId("center");
    center->setBgColor({0.5, 0.0, 0.5, 1});
    auto mainLayout = std::make_shared<AreaLayout>(nullptr, nullptr, nullptr, nullptr, center);
    mainWidget->setLayout(mainLayout);
    mainWidget->addChild(center);
    setTopLevelWidget(mainWidget);

}

void EditorLauncher::doFrame() {
    if (lblFrameTime) {
        lblFrameTime->setText(std::format("FT: {:.1f} ms", Timing::lastFrameTimeInSeconds()*1000) );
    }
}

void EditorLauncher::onFrameMessages(const std::vector<RawWin32Message> &msgs) {
    for (auto rm : msgs) {
        auto transformedMessage = MessageTransformer::transform(rm);
        if (transformedMessage.type == MessageType::MouseMove) {
            lblMouseCoords->setText("mouse: " + std::to_string(transformedMessage.mouseMoveMessage.x) +  "/" + std::to_string(transformedMessage.mouseMoveMessage.y));
        }
    }


}

