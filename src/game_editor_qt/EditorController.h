//
// Created by mgrus on 18.04.2025.
//
#pragma once

#include <QObject>
#include <memory>
#include <qtabwidget.h>

#include "editor_model.h"

class EditorController : public QObject {
    Q_OBJECT

public:
    explicit EditorController(QObject* parent = nullptr);

    void loadProject(const std::string& path);
    bool createNewProject(const QString& name, const QString& path);

    void saveCurrentScript();

    void openScriptInTab(const QString& scriptPath);

    void setAssetTargetTabPanel(QTabWidget* assetTabWidget);

    void saveProjectToJsonFile(edqt::Project *project);

    edqt::Project* currentProject() const { return currentProject_.get(); }
    edqt::Level* currentLevel() const { return currentLevel_.get(); }

    signals:
        void projectChanged(edqt::Project* project);
        void levelChanged(edqt::Level* level);

private:
    std::unique_ptr<edqt::Project> currentProject_;
    std::unique_ptr<edqt::Level> currentLevel_;
    QTabWidget * asset_tab_widget_ = nullptr;
    std::map<QString, QString> tab_fullpath_map_;
};

