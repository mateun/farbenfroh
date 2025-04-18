//
// Created by mgrus on 18.04.2025.
//

#ifndef GAMEOBJECTTREEWIDGET_H
#define GAMEOBJECTTREEWIDGET_H

#include <QTreeWidget>

#include "editor_model.h"


class GameObjectTreeWidget : public QTreeWidget {

  public:
    GameObjectTreeWidget(QWidget* parent = nullptr);

    QTreeWidgetItem *setGenRoot();

    void showContextMenu(const QPoint &pos);

    void setLevel(edqt::Level* level);

private:
    edqt::Level* level_ = nullptr;

};



#endif //GAMEOBJECTTREEWIDGET_H
