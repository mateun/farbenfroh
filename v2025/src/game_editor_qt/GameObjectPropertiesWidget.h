//
// Created by mgrus on 19.04.2025.
//

#ifndef GAMEOBJECTPROPERTIESWIDGET_H
#define GAMEOBJECTPROPERTIESWIDGET_H

#include <QWidget>

namespace edqt {
    struct GameObject;
}



/**
* Shows the properties of the currently selected game object.
* Basically shows a list of all components and their respective props.
*/
class GameObjectPropertiesWidget : public QWidget {

public:
    GameObjectPropertiesWidget(QWidget* parent = nullptr);

    void setGameObject(edqt::GameObject* gameObject);

};



#endif //GAMEOBJECTPROPERTIESWIDGET_H
