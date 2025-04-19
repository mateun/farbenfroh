//
// Created by mgrus on 19.04.2025.
//

#include "GameObjectPropertiesWidget.h"

#include <QWidget>
#include <QVBoxLayout>

#include "editor_model.h"

GameObjectPropertiesWidget::GameObjectPropertiesWidget(QWidget *parent) : QWidget(parent) {
    auto layout = new QVBoxLayout();
    layout->setSpacing(5);
    setLayout(layout);

}

/**
 * This method allows to provide us with the current GameObject.
 * @param gameObject Our main data input. We show all the components and their props.
 */
void GameObjectPropertiesWidget::setGameObject(edqt::GameObject *gameObject) {
    qDebug() << "GameObjectPropertiesWidget::setGameObject called for " << gameObject->name;

    for (auto& comp : gameObject->components) {
        // TODO render the components properties
        // Maybe delegate to the actual component?!
        // Classic textbook polymorphic use-case here :)!
    }



}
