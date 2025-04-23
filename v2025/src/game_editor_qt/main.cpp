//
// Created by mgrus on 16.04.2025.
//

#include <QApplication>
#include <QMainWindow>
#include <QStyleFactory>
#include <QLabel>
#include <QVBoxLayout>
#include <QLabel>
#include <QSplitter>
#include <QImageReader>
#include <QToolBar>
#include <QMainWindow>
#include <QTreeWidget>
#include <QFormLayout>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QListView>
#include <QStandardItemModel>
#include <engine/dx11/dx11_api.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "AssetBrowserWidget.h"
#include "D3DViewPortWidget.h"
#include "EditorController.h"
#include "GameObjectTreeWidget.h"
#include "ProjectDash.h"
#include <QShortcut>
#include <QKeySequence>
#include <engine/actor_model/script_runtime.h>

#include "GameObjectPropertiesWidget.h"

static QWidget* leftPanel;
static GameObjectPropertiesWidget* rightPanel;
static QTabWidget* centerPanel;
static AssetBrowserWidget* assetBrowser_;
static GameObjectTreeWidget* gameObjectTreeWidget_;

void setDarkTheme(QApplication* app) {
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(30, 30, 30));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(20, 20, 20));
    darkPalette.setColor(QPalette::AlternateBase, QColor(30, 30, 30));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(45, 45, 45));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Highlight, QColor(90, 90, 90));
    darkPalette.setColor(QPalette::HighlightedText, Qt::white);

    app->setPalette(darkPalette);
}

void addActionsToToolbar(QToolBar* toolbar) {
    auto newGameAction = new QAction();
    newGameAction->setIcon(QIcon("../assets/icon_new_game.svg"));
    auto saveGameAction = new QAction();
    saveGameAction->setIcon(QIcon("../assets/icon_save_game.svg"));
    auto startGameAction = new QAction();
    startGameAction->setIcon(QIcon("../assets/icon_play.svg"));
    auto pauseGameAction = new QAction();
    pauseGameAction->setIcon(QIcon("../assets/icon_pause.svg"));
    auto stopGameAction = new QAction();
    stopGameAction->setIcon(QIcon("../assets/icon_stop.svg"));

    toolbar->addAction(newGameAction);
    toolbar->addAction(saveGameAction);
    toolbar->addAction(startGameAction);
    toolbar->addAction(pauseGameAction);
    toolbar->addAction(stopGameAction);

}

void createThreeMainPanels(QWidget* centralWidget) {

    leftPanel = new QWidget();
    auto leftLayout = new QVBoxLayout();
    leftLayout->setContentsMargins(0, 0, 0, 0); // tight!
    leftPanel->setLayout(leftLayout);
    leftPanel->setMinimumWidth(120);
    leftPanel->setStyleSheet("background-color: #222; color: white;");

    centerPanel = new QTabWidget();
    centerPanel->setStyleSheet("background-color: #111; color: white;");

    rightPanel = new GameObjectPropertiesWidget();

    rightPanel->setMinimumWidth(160);
    // rightPanel->setStyleSheet("background-color: #222; color: white;");

    QSplitter* horizontalSplitter = new QSplitter(Qt::Horizontal);
    horizontalSplitter->addWidget(leftPanel);
    horizontalSplitter->addWidget(centerPanel);
    horizontalSplitter->addWidget(rightPanel);
    horizontalSplitter->setStretchFactor(1, 1); // center stretches

    // === 2. Bottom Tabbed Panel
    QTabWidget* bottomTabs = new QTabWidget();
    assetBrowser_ = new AssetBrowserWidget();
    bottomTabs->addTab(assetBrowser_, "Asset Browser");
    bottomTabs->setMinimumHeight(150);

    // === Vertical splitter: top 3-panel area + bottom asset area ===
    QSplitter* verticalSplitter = new QSplitter(Qt::Vertical);
    verticalSplitter->addWidget(horizontalSplitter);
    verticalSplitter->addWidget(bottomTabs);
    verticalSplitter->setStretchFactor(0, 4); // top
    verticalSplitter->setStretchFactor(1, 1); // bottom

    // === Set into central layout ===
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(verticalSplitter);

}

void updatePropertiesFor(void* gameObject) {

    QLayoutItem* child;
    while ((child = rightPanel->layout()->takeAt(0)) != nullptr) {
        delete child->widget(); // deletes label + field
        delete child;
    }

    auto layout = (QFormLayout*)rightPanel->layout();

    layout->addRow("Name:", new QLineEdit());
    layout->addRow("Health:", new QSpinBox());
    layout->addRow("AI Enabled:", new QCheckBox());
    layout->addRow("Type:", new QComboBox());
}

void createGameObjectTree(QWidget* targetPanel) {
    gameObjectTreeWidget_= new GameObjectTreeWidget();
    targetPanel->layout()->addWidget(gameObjectTreeWidget_);


}

void initAssetBrowser(QWidget* assetBrowser) {

}

void initViewport(QTabWidget* targetTabWidget) {

    auto viewportWidget = new D3DViewPortWindow();
    QWidget* dxContainer = QWidget::createWindowContainer(viewportWidget);

    dxContainer->setMinimumSize(640, 480);
    targetTabWidget->addTab(dxContainer, "3D Viewport");




}

void clearViewport() {
    dx11_clearBackbuffer({1, 0, 1, 1});
    dx11_presentBackbuffer();
}


int main(int argc, char *argv[]) {
    qputenv("QT_DEBUG_PLUGINS", "1");
    QApplication app(argc, argv);
    setDarkTheme(&app);
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    qDebug() << "Available image formats:" << QImageReader::supportedImageFormats();



    QMainWindow mainWindow;
    mainWindow.setWindowTitle("_Borst Game Editor");
    mainWindow.resize(800, 600);
    QWidget* centralWidget = new QWidget();
    mainWindow.setCentralWidget(centralWidget);

    auto toolbar = new QToolBar();
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(24,24));
    mainWindow.addToolBar(toolbar);

    addActionsToToolbar(toolbar);

    createThreeMainPanels(centralWidget);
    createGameObjectTree(leftPanel);
    //updatePropertiesFor(nullptr);
    initAssetBrowser(assetBrowser_);
    initViewport(centerPanel);

    mainWindow.show();

    auto editor = new EditorController(&app); // Central controller
    // Where the assets should be appended to as tabs when opened
    editor->setAssetTargetTabPanel(centerPanel);
    // Hook components to project/level updates:
    QObject::connect(editor, &EditorController::projectChanged, assetBrowser_, &AssetBrowserWidget::setProject);
    QObject::connect(editor, &EditorController::levelChanged, gameObjectTreeWidget_, &GameObjectTreeWidget::setLevel);
    QObject::connect(assetBrowser_, &AssetBrowserWidget::assetDoubleClicked, editor, &EditorController::openScriptInTab);
    QObject::connect(gameObjectTreeWidget_, &GameObjectTreeWidget::gameObjectSelected, rightPanel, &GameObjectPropertiesWidget::setGameObject);
    // TODO also wire up the 3d viewport to the gameojbect selection, we might outline the currently selected game object there.

    QShortcut* saveShortcut = new QShortcut(QKeySequence("Ctrl+S"), &mainWindow);
    QObject::connect(saveShortcut, &QShortcut::activated, editor, &EditorController::saveCurrentScript);
    // TODO further hooks...

    ProjectDash dialog;
    QObject::connect(&dialog, &ProjectDash::existingProjectChosen, [editor](QString path){
        qDebug() << "Project path selected:" << path;
        editor->loadProject(path.toStdString());

    });
    QObject::connect(&dialog, &ProjectDash::newProjectToBeCreated, [editor, &mainWindow](const QString& name, const QString& path){
        qDebug() << "Project path selected:" << path;
        auto result = editor->createNewProject(name, path);
        if (!result) {
            qFatal() << "Error creating new project" ;
        }
        mainWindow.setWindowTitle("Unnamed Level *");

    });

    // Test vm here for now
    ScriptRuntime runtime;
    auto &ctx = runtime.create_script("player1.behavior");
    auto bc = std::vector<uint8_t> {
        4, 's','e','l','f',           // PUSH_STRING "self"
        4, 'p','i','n','g',           // PUSH_STRING "ping"
        static_cast<uint8_t>(OpCode::SEND),
        4, 'p','i','n','g',           // SEND to func "ping"
        0,                            // 0 args
        static_cast<uint8_t>(OpCode::RETURN)
    };

    Function ping_fn;
    ping_fn.entry_point = 0;
    ping_fn.arg_count = 0;
    runtime.set_function(ctx.id, "ping", ping_fn);
    auto script_definition = ScriptDefinition{
        .code =  bc,
        .functions = std::unordered_map<std::string, Function>{{std::string("ping"), ping_fn}}
    };





    Function on_spawn_fn;
    on_spawn_fn.entry_point = 0;
    on_spawn_fn.arg_count = 0;
    runtime.set_function(ctx.id, "on_spawn", on_spawn_fn);

    runtime.send_message(ctx.id, Message{
        .function_name = "on_spawn",
        .args = {},
        .sender = "engine"
    });
    runtime.tick_all_scripts();
    runtime.tick_all_scripts();

    // end vm test

    // Just block until the dash is closed.
    // If the user chose not to create or load a project,
    // we activate the editor. A project then will need to be choseen via menu or toolbar.
    dialog.exec();

    return app.exec();
}
