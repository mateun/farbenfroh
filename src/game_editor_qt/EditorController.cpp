//
// Created by mgrus on 18.04.2025.
//

#include "EditorController.h"
#include <QDebug>
#include "editor_model.h"

EditorController::EditorController(QObject* parent)
    : QObject(parent) {}

void EditorController::loadProject(const std::string& path) {
    // Load logic here, or stub it
    qDebug() << "Loading project from" << path;

     currentProject_ = std::make_unique<Project>();
     currentProject_->name = "StubProject";
     currentProject_->systemFilePath = path;

    emit projectChanged(currentProject_.get());
}

void EditorController::createNewProject(const std::string& name, const std::string& path) {
    qDebug() << "Creating new project:" << name << "at" << path;

    currentProject_ = std::make_unique<Project>();
    currentProject_->name = name;
    currentProject_->systemFilePath = path;

    // Also create an initial level
    currentLevel_ = std::make_unique<Level>();
    currentLevel_->name = "UntitledLevel";
    currentLevel_->relativePath = "levels/UntitledLevel.json";

    emit projectChanged(currentProject_.get());
    emit levelChanged(currentLevel_.get());
}
