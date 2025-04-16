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

#include "D3DViewPortWidget.h"

static QWidget* leftPanel;
static QWidget* rightPanel;
static QTabWidget* centerPanel;
static QWidget* assetBrowser_;

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

    rightPanel = new QWidget();
    auto* rightLayout = new QFormLayout();
    rightLayout->setLabelAlignment(Qt::AlignRight);
    rightLayout->setFormAlignment(Qt::AlignTop);
    rightPanel->setLayout(rightLayout);
    rightPanel->setMinimumWidth(160);
    rightPanel->setStyleSheet("background-color: #222; color: white;");

    QSplitter* horizontalSplitter = new QSplitter(Qt::Horizontal);
    horizontalSplitter->addWidget(leftPanel);
    horizontalSplitter->addWidget(centerPanel);
    horizontalSplitter->addWidget(rightPanel);
    horizontalSplitter->setStretchFactor(1, 1); // center stretches

    // === 2. Bottom Tabbed Panel
    QTabWidget* bottomTabs = new QTabWidget();
    assetBrowser_ = new QWidget();
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
    auto tree = new QTreeWidget();
    tree->setHeaderHidden(true); // no header if you want that minimal look
    tree->setStyleSheet("background-color: #111; color: white;");

    // Add some test nodes
    auto root = new QTreeWidgetItem(tree, QStringList("Player"));
    new QTreeWidgetItem(root, QStringList("Camera"));
    new QTreeWidgetItem(root, QStringList("Collider"));

    auto enemy = new QTreeWidgetItem(tree, QStringList("Enemy"));
    new QTreeWidgetItem(enemy, QStringList("AI"));
    new QTreeWidgetItem(enemy, QStringList("Health"));

    tree->expandAll(); // for now

    targetPanel->layout()->addWidget(tree);


}

void initAssetBrowser(QWidget* assetBrowser) {
    // TODO add this to the asset browser
    // also add a menu here.

    auto* layout = new QVBoxLayout(assetBrowser);
    // top menu or search bar
    QHBoxLayout* topBar = new QHBoxLayout();
    topBar->addWidget(new QLabel("Assets"));
    topBar->addStretch();
    topBar->addWidget(new QLineEdit("Search..."));
    layout->addLayout(topBar);

    auto listView = new QListView();
    listView->setViewMode(QListView::IconMode);
    listView->setResizeMode(QListView::Adjust);
    listView->setSpacing(10);
    listView->setMovement(QListView::Static);
    listView->setIconSize(QSize(128, 128));
    listView->setUniformItemSizes(true);

    listView->setDragEnabled(true);
    listView->setAcceptDrops(true);
    listView->setDropIndicatorShown(true);
    layout->addWidget(listView);

    // Generate preview
    // QPixmap::fromImage(...)
    // Meshes on our own
    // Sounds we just show an icon

    auto model = new QStandardItemModel();

    QIcon texturePreview("textures/diffuse1.png");
    model->appendRow(new QStandardItem(QIcon("../assets/icon_package.svg"), "Mesh01"));
    model->appendRow(new QStandardItem(QIcon("../assets/icon_play.svg"), "Sound01"));
    model->appendRow(new QStandardItem(QIcon("../assets/icon_save_game.svg"), "Texture01"));


    listView->setModel(model);
}

void initViewport(QTabWidget* targetTabWidget) {


    auto viewportWidget = new D3DViewPortWidget();
    viewportWidget->setMinimumSize(640, 480);
    viewportWidget->setAttribute(Qt::WA_NativeWindow); // required
    viewportWidget->setAttribute(Qt::WA_PaintOnScreen); // optional

    targetTabWidget->addTab(viewportWidget, "3D Viewport");


    // 3. Pass HWND to your DirectX initialization code



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
    updatePropertiesFor(nullptr);
    initAssetBrowser(assetBrowser_);
    initViewport(centerPanel);


    mainWindow.show();

    clearViewport();

    return app.exec();
}
