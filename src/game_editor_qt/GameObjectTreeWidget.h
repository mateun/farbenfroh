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

    void setLevel(Level* level) {
        level_ = level;
    };

private:
    Level* level_;

};



#endif //GAMEOBJECTTREEWIDGET_H
