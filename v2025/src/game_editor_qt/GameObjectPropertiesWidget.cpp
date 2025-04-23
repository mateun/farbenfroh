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

    // Clear the child widgets
    QLayoutItem* child;
    while ((child = layout()->takeAt(0)) != nullptr) {
        delete child->widget(); // deletes label + field
        delete child;
    }

    for (auto& component : gameObject->components) {
        QWidget* propWidget = component->createPropertyWidget(this);
        if (propWidget)
            layout()->addWidget(propWidget);

    }



}
