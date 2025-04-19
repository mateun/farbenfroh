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
#include <QLabel>

ProjectDash::ProjectDash(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Select or Create Project");
    setModal(true);
    resize(500, 450);

    auto* layout = new QVBoxLayout(this);

    // --- Load existing project which is not in the recent list ---
    auto openProjectButton = new QPushButton("Open existing project", this);
    connect(openProjectButton, &QPushButton::clicked, [this]() {
        QString folderName = QFileDialog::getExistingDirectory(this, "Select Project Folder");
        if (validateOpeningPath(folderName)) {
            emit existingProjectChosen(folderName);
            accept();
        }
    });
    layout->addWidget(openProjectButton);

    // --- Recent Projects ---
    layout->addWidget(new QLabel("Recent Projects:"));
    recentList_ = new QListWidget(this);
    layout->addWidget(recentList_);

    // Dummy data
    recentList_->addItem("Sample Project 1 - C:/dev/temp/foobar");
    recentList_->addItem("Sample Project 2 - D:/Games/Test2");

    connect(recentList_, &QListWidget::itemClicked, this, &ProjectDash::onRecentClicked);

    // --- Create New Project ---
    layout->addWidget(new QLabel("Create New Project:"));
    auto* formLayout = new QVBoxLayout();

    nameEdit_ = new QLineEdit(this);
    nameEdit_->setPlaceholderText("Project name");
    connect(nameEdit_, &QLineEdit::textChanged, [this] {
        validateCreationPath();
    });

    pathEdit_ = new QLineEdit(this);
    pathEdit_->setPlaceholderText("Choose project folder");
    connect(pathEdit_, &QLineEdit::textEdited, [this] {
        validateCreationPath();
    });
    error_label_ = new QLabel(this);
    error_label_->hide();

    browseButton_ = new QPushButton("Browse...", this);
    connect(browseButton_, &QPushButton::clicked, [this]() {
        QString folderName = QFileDialog::getExistingDirectory(this, "Select Project Folder");
        if (!folderName.isEmpty()) {
            pathEdit_->setText(folderName);
            validateCreationPath();
        }
    });

    createButton_ = new QPushButton("Create Project", this);
    createButton_->setStyleSheet(R"(
        QPushButton {
            background-color: #444444;
            color: white;
            border: 1px solid #777777;
            padding: 5px;
        }
        QPushButton:disabled {
            color: #777777;
            background-color: #2a2a2a;
            border: 1px solid #444444;
        }
    )");

    createButton_->setDisabled(true);
    connect(createButton_, &QPushButton::clicked, this, &ProjectDash::onCreateProject);

    auto* pathRow = new QHBoxLayout();
    pathRow->addWidget(pathEdit_);
    pathRow->addWidget(browseButton_);

    auto *pathErrorRow = new QHBoxLayout();
    pathErrorRow->addWidget(error_label_);

    formLayout->addWidget(nameEdit_);
    formLayout->addLayout(pathRow);
    formLayout->addLayout(pathErrorRow);
    formLayout->addWidget(createButton_);

    layout->addLayout(formLayout);
}

bool ProjectDash::validateOpeningPath(const QString& folderName) const {
    bool valid = true;
    QString errorText ="";
    if (!folderName.isEmpty()) {
        // check if we have a .project.json file
        auto f = QFile (folderName + "/.project.json");
        if (!f.exists()) {
            valid = false;
            errorText = "Project file not found! This seems not to be a valid project folder.";

        }

    } else {
        errorText = "No project folder given!";
        valid = false;
    }

    if (valid) {
        error_label_->hide();
    } else {
        error_label_->setText(errorText);
        error_label_->setStyleSheet("color: red;");
        error_label_->show();
    }

    return valid;
}

bool ProjectDash::validateCreationPath() const {
    bool valid = true;
    QString errorText ="";
    // Validate this path. If it exists and has already a .project.json in it, we refuse.
    if (!pathEdit_->text().isEmpty()) {
        auto folderName = pathEdit_->text();
        auto full_path = folderName + "/" + nameEdit_->text();
        auto folder = QDir(full_path);
        if (folder.exists()) {
            auto projectJson = QFile(full_path   + "/.project.json");
            if (projectJson.exists()) {
                errorText = "A project already exists in this folder.";
                valid = false;
            }
        }
    } else {
        errorText = "A project location must be provided.";
        valid = false;
    }
    if (nameEdit_->text().isEmpty()) {
        valid = false;
        errorText += "The name must be provided.";
    }

    if (valid) {
        pathEdit_->setStyleSheet(""); // Resets to default appearance
        createButton_->setEnabled(true);
        error_label_->hide();
    }else {
        pathEdit_->setStyleSheet("border: 1px solid red;");
        pathEdit_->setToolTip(errorText);
        error_label_->setText(errorText);
        error_label_->setStyleSheet("color: red;");
        error_label_->show();
        createButton_->setEnabled(false);
    }

    return valid;

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
    if (!validateCreationPath()) {
        return;
    }

    selectedPath_ = pathEdit_->text();
    emit newProjectToBeCreated(nameEdit_->text(), selectedPath_);
    accept(); // close dialog

}
