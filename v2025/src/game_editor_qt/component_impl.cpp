//
// Created by mgrus on 19.04.2025.
//
#include <QFormLayout>
#include <QLineEdit>

#include "editor_model.h"
#include <QWidget>

QWidget *edqt::TransformComponent::createPropertyWidget(GameObjectPropertiesWidget *game_object_properties_widget) {
    QWidget *widget = new QWidget();
    auto* formLayout = new QFormLayout();
    formLayout->setLabelAlignment(Qt::AlignRight);
    formLayout->setFormAlignment(Qt::AlignTop);
    widget->setLayout(formLayout);

    formLayout->addRow("Position:", new QLineEdit());
    formLayout->addRow("Rotation:", new QLineEdit());
    formLayout->addRow("Scale:", new QLineEdit());

    return widget;

}
