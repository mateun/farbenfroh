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
#include <QInputDialog>
#include <QMessageBox>

#include "editor_model.h"
#include "ProjectDash.h"

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

    connect(folderTree_, &QTreeWidget::customContextMenuRequested,
            this, &AssetBrowserWidget::showFolderContextMenu);
    connect(folderTree_, &QTreeWidget::itemClicked,
        this, &AssetBrowserWidget::onFolderSelected);

    // Enable dynamic expansion:
    connect(folderTree_, &QTreeWidget::itemExpanded, this, [=](QTreeWidgetItem* item) {
        // Only load if dummy child present
        if (item->childCount() == 1 && item->child(0)->text(0).isEmpty()) {
            item->removeChild(item->child(0)); // remove dummy

            QString path = item->data(0, Qt::UserRole).toString();
            QDir dir(path);
            QStringList subFolders = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

            for (const QString& folder : subFolders) {
                QString fullPath = dir.absoluteFilePath(folder);
                QTreeWidgetItem* subItem = new QTreeWidgetItem(item, QStringList(folder));
                subItem->setData(0, Qt::UserRole, fullPath);

                if (QDir(fullPath).entryList(QDir::Dirs | QDir::NoDotAndDotDot).size() > 0) {
                    subItem->addChild(new QTreeWidgetItem()); // add dummy again
                }
            }
        }
    });

    // Our asset list viewer:
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
    connect(assetList_, &QListView::doubleClicked, this, &AssetBrowserWidget::onAssetDoubleClicked);

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

void AssetBrowserWidget::onAssetDoubleClicked(const QModelIndex& index) {
    QString assetName = index.data(Qt::DisplayRole).toString();
    QString assetPath = QString::fromStdString(currentFolderPath_) + "/" + assetName;
    qDebug() << "Double-clicked asset:" << assetPath;

    // Emit signal to open it in a new tab, or do it directly:
    emit assetDoubleClicked(assetPath);
}


void AssetBrowserWidget::refreshAssetListFor(const std::string &folderPath) {
    auto assets = QDir(QString::fromStdString(folderPath)).entryList(QDir::Files);
    asset_list_model_->clear();
    for (auto asset: assets) {
        QString suffix = QFileInfo(asset).suffix().toLower();

        QIcon icon;
        if (suffix == "lua") {
            icon = QIcon("../assets/icon_sourcefile.svg");
        } else if (suffix == "glb" || suffix == "obj") {
            icon = QIcon(":/icons/icon_mesh.svg");
        } else if (suffix == "png" || suffix == "jpg") {
            icon = QIcon(":/icons/icon_texture.svg");
        } else {
            icon = QIcon(":/icons/icon_default.svg"); // fallback
        }
        asset_list_model_->appendRow(new QStandardItem(icon, asset));

    }
}

void AssetBrowserWidget::onFolderSelected(QTreeWidgetItem* item, int column) {
    QString folderPath = item->data(0, Qt::UserRole).toString();  // or item->text(0) if that's your folder name
    currentFolderPath_ = folderPath.toStdString();

    // Refresh the right-side asset list view based on this folder
    refreshAssetListFor(currentFolderPath_);
}


void AssetBrowserWidget::showFolderContextMenu(const QPoint& pos) {
    QTreeWidgetItem* item = folderTree_->itemAt(pos);
    if (!item) return;


    QString relativePath;
    QTreeWidgetItem* current = item;
    while (current && current->parent()) {
        relativePath = current->text(0) + "/" + relativePath;
        current = current->parent();
    }
    relativePath = "Assets/" + relativePath; // Assuming "Assets" is the root
    QString projectRoot = QString::fromStdString(project_->systemFilePath);
    QString fullPath = QDir(projectRoot).filePath(relativePath);
    QDir baseDir(fullPath); // wherever your current folder is


    QMenu menu(this);
    auto newAssetSubMenu = menu.addMenu("New ... ");
    QAction* createAction = newAssetSubMenu->addAction("Folder");
    QAction* createScriptAction = newAssetSubMenu->addAction("Script");
    QAction* createMaterialAction = newAssetSubMenu->addAction("Material");
    menu.addSeparator();
    auto importSubMenu = menu.addMenu("Import...");
    QAction* importMeshAction = importSubMenu->addAction("Mesh");
    QAction* importTextureAction = importSubMenu->addAction("Texture");
    menu.addSeparator();
    QAction* deleteAction = menu.addAction("Delete Folder");



    QAction* chosen = menu.exec(folderTree_->viewport()->mapToGlobal(pos));

    if (chosen == createAction) {
        // Call your new-folder logic
        qDebug() << "Creating folder under " << item;
        bool ok;
        QString folderName = QInputDialog::getText(
            this,
            "Create New Folder",
            "Folder Name:",
            QLineEdit::Normal,
            "",
            &ok
        );

        if (ok && !folderName.isEmpty()) {

            if (!baseDir.mkpath(folderName)) {
                QMessageBox::warning(this, "Error", "Failed to create folder.");
            } else {

                new QTreeWidgetItem(item, QStringList(folderName));
                folderTree_->expandItem(item);
            }
        }

    } else if (chosen == deleteAction) {
        // Delete logic here
    } else if (chosen == createScriptAction) {
        bool ok;
        QString fileName = QInputDialog::getText(
            this,
            "Create New Script",
            "File Name:",
            QLineEdit::Normal,
            "",
            &ok
        );

        if (ok && !fileName.isEmpty()) {

            auto info = QFileInfo(fullPath + "/" + fileName);
            if (info.suffix().isEmpty()) {
                fileName += ".lua";

            }
            auto newFile = new QFile(fullPath + "/" + fileName);
            newFile->open(QIODevice::WriteOnly);
            newFile->close();

        }
    }
}

/**
 * This method gets called when a new project is set in the editor.
 * Our task here is to reload the assets and examine the file system at the projects location.
 * @param project The new now "current" project.
 */
void AssetBrowserWidget::setProject(edqt::Project *project) {
    // folderTree_->clear();
    // auto root  = new QTreeWidgetItem(folderTree_, QStringList("Assets"));
    // project_ = project;
    // auto subFolders = QDir(QString::fromStdString(project_->systemFilePath) + "/Assets").entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    // for (auto f : subFolders) {
    //     new QTreeWidgetItem(root, QStringList(f));
    // }

    folderTree_->clear();
    project_ = project;

    auto root = new QTreeWidgetItem(folderTree_, QStringList("Assets"));
    root->setData(0, Qt::UserRole, QString::fromStdString(project_->systemFilePath) + "/Assets");
    folderTree_->addTopLevelItem(root);

    QDir baseDir(QString::fromStdString(project_->systemFilePath) + "/Assets");
    QStringList subFolders = baseDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString& folder : subFolders) {
        QString fullPath = baseDir.absoluteFilePath(folder);
        QTreeWidgetItem* item = new QTreeWidgetItem(root, QStringList(folder));
        item->setData(0, Qt::UserRole, fullPath);

        // Lazy loading: add dummy if subfolders exist
        if (QDir(fullPath).entryList(QDir::Dirs | QDir::NoDotAndDotDot).size() > 0) {
            item->addChild(new QTreeWidgetItem()); // dummy
        }
    }

    root->setExpanded(true);

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