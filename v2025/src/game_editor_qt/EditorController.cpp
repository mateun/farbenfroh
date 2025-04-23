//
// Created by mgrus on 18.04.2025.
//

#include "EditorController.h"

#include <fstream>
#include <QDebug>
#include <qdir.h>
#include <QFile>
#include <QMessageBox>
#include <QTextEdit>
#include <qtreewidget.h>
#include <engine/io/nljson.h>

#include "editor_model.h"

EditorController::EditorController(QObject* parent)
    : QObject(parent) {}

void EditorController::loadProject(const std::string& path) {
    // Load logic here, or stub it
    qDebug() << "Loading project from" << path;

    QFile file(QString::fromStdString(path) + "/.project.json");
    if (file.open(QIODevice::ReadOnly)) {
        auto data = file.readAll();
        file.close();
        try {
            nlohmann::json json = nlohmann::json::parse(data);
            currentProject_ = std::make_unique<edqt::Project>();
            currentProject_->name = json["name"].get<std::string>();
            currentProject_->systemFilePath = path;

        } catch (const nlohmann::json::exception& e) {
            qCritical() << "Failed to parse project file" << e.what();
        }

    } else {
        qFatal() << "Failed to open project file at: " << path;

    }

    // Level loading:
    // We first try to load the "editor_startup_level" as a convenience.
    // If this fails or is empty, we load the first level we
    // find in the projects level folder.
    // If this also fails, we just create a dummy in-memory level and use that as a fallback.
    // The user might have created the project, but not yet saved a level, which is fine,
    // we should handle this gracefully.
    // First check, do we even have any levels yet?
    // Otherwise we do not need to look for the editor_startup_level or any other level to load, and
    // we can create an empty one right away:
    auto levelsDirectory = QDir(QString::fromStdString(currentProject_->systemFilePath) + "/Levels");
    if (!levelsDirectory.exists()) {
        currentLevel_ = std::make_unique<edqt::Level>();
    } else {
        if (!currentProject_->editor_startup_level.empty()) {
            auto levelDir = QDir(QString::fromStdString(currentProject_->systemFilePath) + "/Levels/" + QString::fromStdString(currentProject_->editor_startup_level));
            if (levelDir.exists()) {
                auto current_level_ = loadLevelFromPath(levelDir.path());
            } else {
                qWarning() << "Failed to load editor level from" << levelDir.path();
                if (levelsDirectory.entryList(QDir::Dirs).size() > 0) {
                    currentLevel_ = loadLevelFromPath(levelsDirectory.entryList(QDir::Dirs)[0]);
                }
            }
        }
    }


    emit projectChanged(currentProject_.get());
    emit levelChanged(currentLevel_.get());
}

bool EditorController::createNewProject(const QString& name, const QString& path) {
    qDebug() << "Creating new project:" << name << "at" << path;


    // We assume every validation check has already been done so here
    // we just naively create the folder if it does not exist and then put our json file(s) in there:
    auto folderPath =  path + "/" + name;
    QDir dir;
    if (!dir.exists(folderPath)) {
        bool success = dir.mkpath(folderPath); // Creates all parent folders if needed
        if (!success) {
            qErrnoWarning("Failed to create project folder");
            return false;
        }
    }

    currentProject_ = std::make_unique<edqt::Project>();
    currentProject_->name = name.toStdString();
    currentProject_->systemFilePath = folderPath.toStdString();
    auto assetFolderOK = QDir().mkpath(folderPath + "/Assets");
    if (!assetFolderOK) {
        return false;
    }


    saveProjectToJsonFile(currentProject_.get());

    // Also create an initial level, which is not yet saved to disk of now.
    // So we provide an in-memory level for the user to fill, but
    // we should always mark the current level as unsaved so the user shall save it.
    currentLevel_ = std::make_unique<edqt::Level>();
    currentLevel_->name = "UntitledLevel";
    currentLevel_->gameObjects = {};
    //currentLevel_->relativePath = "levels/UntitledLevel.json";


    emit projectChanged(currentProject_.get());
    emit levelChanged(currentLevel_.get());
    return true;
}
void EditorController::saveCurrentScript() {
    QWidget* current = asset_tab_widget_->currentWidget();
    QTextEdit* editor = qobject_cast<QTextEdit*>(current);
    if (!editor) return;

    QString tabText = asset_tab_widget_->tabText(asset_tab_widget_->currentIndex());
    QString cleanName = tabText.endsWith("*") ? tabText.left(tabText.length() - 1) : tabText;
    QString filePath = tab_fullpath_map_[cleanName];

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << editor->toPlainText();
        file.close();
        asset_tab_widget_->setTabText(asset_tab_widget_->currentIndex(), cleanName); // Remove *
    } else {
        QMessageBox::warning(nullptr, "Error", "Failed to save file.");
    }
}


void EditorController::openScriptInTab(const QString &scriptPath) {
    qDebug("received task to open a new lua script in our main tab control");

    // Check if the script is already open in a tab
    for (int i = 0; i < asset_tab_widget_->count(); ++i) {
        if (asset_tab_widget_->tabText(i) == QFileInfo(scriptPath).fileName()) {
            asset_tab_widget_->setCurrentIndex(i); // Focus that tab
            return;
        }
    }

    // Not found — create a new tab
    QTextEdit* editor = new QTextEdit(); // Replace this with a better (scintilla?) editor later.

    connect(editor, &QTextEdit::textChanged, this, [=]() {
    int tabIndex = asset_tab_widget_->indexOf(editor);
    QString tabText = asset_tab_widget_->tabText(tabIndex);
    if (!tabText.endsWith("*")) {
        asset_tab_widget_->setTabText(tabIndex, tabText + "*");
    }
    });

    QFile file(scriptPath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        editor->setPlainText(file.readAll());
        file.close();
    } else {
        editor->setPlainText("// Could not open file");
    }

    QString tabName = QFileInfo(scriptPath).fileName();
    tab_fullpath_map_[tabName] = scriptPath;
    asset_tab_widget_->addTab(editor, tabName);
    asset_tab_widget_->setCurrentWidget(editor); // Focus the new tab
}

void EditorController::setAssetTargetTabPanel(QTabWidget *assetTabWidget) {
    asset_tab_widget_ = assetTabWidget;
}

std::unique_ptr<edqt::Level> EditorController::loadLevelFromPath(const QString& path) {
    QFile file(path + "/.level.json");
    if (file.open(QIODevice::ReadOnly)) {
        auto data = file.readAll();
        file.close();
        try {
            nlohmann::json json = nlohmann::json::parse(data);
            auto level = std::make_unique<edqt::Level>();
            level->name = json["name"].get<std::string>();
            level->relativePath = QFileInfo(path).fileName().toStdString();
            // TODO load gameobject data...

            return level;

        } catch (const nlohmann::json::exception& e) {
            qCritical() << "Failed to parse project file" << e.what();
        }

    } else {
        qFatal() << "Failed to open project file at: " << path;

    }
}

void EditorController::saveProjectToJsonFile(edqt::Project* project) {
    using namespace nlohmann;
    json j;

    j["name"] = project->name;

    // Optional: store levels and assets as filenames
    j["levels"] = json::array();
    // for (const auto* level : project->le) {
    //     j["levels"].push_back(level->filePath);
    // }

    j["assets"] = json::array();
    // for (const auto* asset : project.assets) {
    //     j["assets"].push_back(asset->filePath);
    // }

    j["last_open_level"] = "";

    std::string outputPath = project->systemFilePath + "/.project.json";
    std::ofstream outFile(outputPath);
    if (outFile) {
        outFile << j.dump(4); // pretty print with 4 spaces
        outFile.close();
        qDebug() << "Project saved to:" << QString::fromStdString(outputPath);
    } else {
        qWarning() << "Failed to write project file to:" << QString::fromStdString(outputPath);
    }
}
