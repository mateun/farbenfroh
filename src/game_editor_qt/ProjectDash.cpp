//
// Created by mgrus on 17.04.2025.
//

#include "ProjectDash.h"


#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QLabel>

ProjectDash::ProjectDash(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Select or Create Project");
    setModal(true);
    resize(500, 400);

    auto* layout = new QVBoxLayout(this);

    // --- Recent Projects ---
    layout->addWidget(new QLabel("Recent Projects:"));
    recentList_ = new QListWidget(this);
    layout->addWidget(recentList_);

    // Dummy data
    recentList_->addItem("Sample Project 1 - C:/Projects/Game1");
    recentList_->addItem("Sample Project 2 - D:/Games/Test2");

    connect(recentList_, &QListWidget::itemClicked, this, &ProjectDash::onRecentClicked);

    // --- Create New Project ---
    layout->addWidget(new QLabel("Create New Project:"));
    auto* formLayout = new QVBoxLayout();

    nameEdit_ = new QLineEdit(this);
    nameEdit_->setPlaceholderText("Project name");

    pathEdit_ = new QLineEdit(this);
    pathEdit_->setPlaceholderText("Choose project folder");

    browseButton_ = new QPushButton("Browse...", this);
    connect(browseButton_, &QPushButton::clicked, [this]() {
        QString folder = QFileDialog::getExistingDirectory(this, "Select Project Folder");
        if (!folder.isEmpty()) {
            pathEdit_->setText(folder);
        }
    });

    createButton_ = new QPushButton("Create Project", this);
    connect(createButton_, &QPushButton::clicked, this, &ProjectDash::onCreateProject);

    auto* pathRow = new QHBoxLayout();
    pathRow->addWidget(pathEdit_);
    pathRow->addWidget(browseButton_);

    formLayout->addWidget(nameEdit_);
    formLayout->addLayout(pathRow);
    formLayout->addWidget(createButton_);

    layout->addLayout(formLayout);
}

QString ProjectDash::selectedProjectPath() const {
    return selectedPath_;
}

QString ProjectDash::projectName() const {
    return nameEdit_->text();
}

void ProjectDash::onRecentClicked(QListWidgetItem* item) {
    selectedPath_ = item->text().section(" - ", 1); // crude parsing
    emit existingProjectChosen(selectedPath_);
    accept(); // close dialog
}

void ProjectDash::onCreateProject() {
    if (nameEdit_->text().isEmpty() || pathEdit_->text().isEmpty())
        return;

    selectedPath_ = pathEdit_->text() + "/" + nameEdit_->text();
    emit existingProjectChosen(selectedPath_);
    accept(); // close dialog
}
