//
// Created by mgrus on 18.04.2025.
//
#pragma once

#include <QObject>
#include <memory>
#include "editor_model.h"

class EditorController : public QObject {
    Q_OBJECT

public:
    explicit EditorController(QObject* parent = nullptr);

    void loadProject(const std::string& path);
    void createNewProject(const std::string& name, const std::string& path);

    Project* currentProject() const { return currentProject_.get(); }
    Level* currentLevel() const { return currentLevel_.get(); }

    signals:
        void projectChanged(Project* project);
        void levelChanged(Level* level);

private:
    std::unique_ptr<Project> currentProject_;
    std::unique_ptr<Level> currentLevel_;
};

