//
// Created by mgrus on 17.04.2025.
//

#include "AssetBrowserWidget.h"

#include <qdir.h>
#include <QFile>
#include <QDropEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMenu>
#include <QMimeData>
#include <QSplitter>
#include <QStandardItemModel>
#include <QTreeWidget>

AssetBrowserWidget::AssetBrowserWidget(QWidget *parent) : QWidget(parent) {
    setAcceptDrops(true);

    auto* layout = new QVBoxLayout(this);
    // top menu or search bar
    QHBoxLayout* topBar = new QHBoxLayout();
    topBar->addWidget(new QLabel("Assets"));
    topBar->addStretch();
    topBar->addWidget(new QLineEdit("Search..."));
    layout->addLayout(topBar);

    auto *splitter = new QSplitter(Qt::Horizontal, this);
    folderTree_ = new QTreeWidget();
    folderTree_->setHeaderLabel("Folders");
    folderTree_->setMinimumWidth(180);
    folderTree_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    folderTree_->setContextMenuPolicy(Qt::CustomContextMenu);
    auto root = new QTreeWidgetItem(folderTree_, QStringList("Assets"));
    root->setExpanded(true);
    connect(folderTree_, &QTreeWidget::customContextMenuRequested,
            this, &AssetBrowserWidget::showFolderContextMenu);
    connect(folderTree_, &QTreeWidget::itemClicked,
        this, &AssetBrowserWidget::onFolderSelected);


    // Add some test folders here
    new QTreeWidgetItem(root, QStringList("Scripts"));
    new QTreeWidgetItem(root, QStringList("Meshes"));
    new QTreeWidgetItem(root, QStringList("Textures"));
    new QTreeWidgetItem(root, QStringList("Materials"));


    assetList_ = new QListView();
    assetList_->setViewMode(QListView::IconMode);
    assetList_->setResizeMode(QListView::Adjust);
    assetList_->setSpacing(10);
    assetList_->setMovement(QListView::Static);
    assetList_->setIconSize(QSize(128, 128));
    assetList_->setUniformItemSizes(true);

    assetList_->setDragEnabled(true);
    assetList_->setAcceptDrops(true);
    assetList_->setDropIndicatorShown(true);
    splitter->addWidget(folderTree_);
    splitter->addWidget(assetList_);
    splitter->setStretchFactor(1, 1);


    layout->addWidget(splitter);

    // Generate preview
    // QPixmap::fromImage(...)
    // Meshes on our own
    // Sounds we just show an icon

    asset_list_model_ = new QStandardItemModel();

    QIcon texturePreview("textures/diffuse1.png");
    asset_list_model_->appendRow(new QStandardItem(QIcon("../assets/icon_package.svg"), "Mesh01"));
    asset_list_model_->appendRow(new QStandardItem(QIcon("../assets/icon_play.svg"), "Sound01"));
    asset_list_model_->appendRow(new QStandardItem(QIcon("../assets/icon_save_game.svg"), "Texture01"));

    assetList_->setModel(asset_list_model_);
}

void AssetBrowserWidget::refreshAssetListFor(const std::string &folderPath) {
    auto assets = QDir("assets/myfolder").entryList();
    asset_list_model_->clear();
}

void AssetBrowserWidget::onFolderSelected(QTreeWidgetItem* item, int column) {
    QString folderPath = item->data(0, Qt::UserRole).toString();  // or item->text(0) if that's your folder name
    currentFolderPath_ = folderPath.toStdString();

    // Refresh the right-side asset list view based on this folder
    refreshAssetListFor(currentFolderPath_);
}


void AssetBrowserWidget::showFolderContextMenu(const QPoint& pos) {
    QTreeWidgetItem* item = folderTree_->itemAt(pos);

    QMenu menu(this);
    QAction* createAction = menu.addAction("New Folder");
    QAction* deleteAction = menu.addAction("Delete Folder");
    QAction* chosen = menu.exec(folderTree_->viewport()->mapToGlobal(pos));

    if (chosen == createAction) {
        // Call your new-folder logic
    } else if (chosen == deleteAction) {
        // Delete logic here
    }
}

/**
 * This method gets called when a new project is set in the editor.
 * Our task here is to reload the assets and examine the file system at the projects location.
 * @param project The new now "current" project.
 */
void AssetBrowserWidget::setProject(Project *project) {
    project_ = project;
    // For now we expect a fixed folder hierarchy in a project to load assets from:
    // the /assets folder holds all physical assets flat.
    // But we also have virtual folders. each asset in the project.json has a virtual folder, so we need
    // to respect that as well.

}

void AssetBrowserWidget::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) {
        const auto urls = event->mimeData()->urls();
        for (const QUrl& url : urls) {
            if (url.toLocalFile().endsWith(".glb", Qt::CaseInsensitive)) {
                event->acceptProposedAction();
                return;
            }
        }
    }
    event->ignore();
}

void AssetBrowserWidget::dropEvent(QDropEvent* event) {
    const QList<QUrl> urls = event->mimeData()->urls();
    for (const QUrl& url : urls) {
        QString path = url.toLocalFile();
        if (path.endsWith(".glb", Qt::CaseInsensitive)) {
            QFile file(path);
            if (file.open(QIODevice::ReadOnly)) {
                QByteArray glbData = file.readAll();
                file.close();
                qDebug() << "GLB file dropped, size:" << glbData.size();

                // 🔥 Your code to forward this binary data into your engine
                // maybe emit a signal: emit glbDropped(glbData, path);
            }
        }
    }
}