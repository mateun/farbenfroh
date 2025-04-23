//
// Created by mgrus on 17.04.2025.
//

#ifndef PROJECTDASH_H
#define PROJECTDASH_H


#include <QDialog>
#include <qlabel.h>
#include <QString>
#include <QVector>
#include <QListWidget>

class QLineEdit;
class QPushButton;



/**
* Is displayed when the editor has no "current project", so
* normally this would be on a fresh startup, when no "favorite" last project is availble and pre-loaded.
*/
class ProjectDash : public QDialog {
    Q_OBJECT
public:
    explicit ProjectDash(QWidget *parent = nullptr);

    bool validateOpeningPath(const QString& folderName) const;

    bool validateCreationPath() const;

    QString selectedProjectPath() const;
    QString projectName() const;


    signals:
        void existingProjectChosen(QString path);
        void newProjectToBeCreated(const QString& name, const QString& path);


    private slots:
        void onRecentClicked(QListWidgetItem* item);
    void onCreateProject();

private:
    QListWidget* recentList_;
    QLineEdit* nameEdit_;
    QLineEdit* pathEdit_;
    QPushButton* browseButton_;
    QPushButton* createButton_;

    QString selectedPath_;
    QLabel * error_label_ = nullptr;
};



#endif //PROJECTDASH_H
