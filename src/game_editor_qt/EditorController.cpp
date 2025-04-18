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
#include <engine/io/nljson.h>

#include "editor_model.h"

EditorController::EditorController(QObject* parent)
    : QObject(parent) {}

void EditorController::loadProject(const std::string& path) {
    // Load logic here, or stub it
    qDebug() << "Loading project from" << path;

     currentProject_ = std::make_unique<edqt::Project>();
     currentProject_->name = "StubProject";
     currentProject_->systemFilePath = path;

    emit projectChanged(currentProject_.get());
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

    // Also create an initial level
    currentLevel_ = std::make_unique<edqt::Level>();
    currentLevel_->name = "UntitledLevel";
    currentLevel_->relativePath = "levels/UntitledLevel.json";
    // TODO and think about the new "empty shell" level, what to do and when to save it etc.

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
