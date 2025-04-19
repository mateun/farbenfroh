//
// Created by mgrus on 18.04.2025.
//

#ifndef GAMEOBJECTTREEWIDGET_H
#define GAMEOBJECTTREEWIDGET_H

#include <QTreeWidget>

#include "editor_model.h"


class GameObjectTreeWidget : public QTreeWidget {
    Q_OBJECT
  public:
    GameObjectTreeWidget(QWidget* parent = nullptr);

    void selectionChanged();

    QTreeWidgetItem *setGenRoot();

    void showContextMenu(const QPoint &pos);

    void setLevel(edqt::Level* level);


signals:
    void gameObjectSelected(edqt::GameObject* gameObject);

private:
    edqt::Level* level_ = nullptr;

};



#endif //GAMEOBJECTTREEWIDGET_H
