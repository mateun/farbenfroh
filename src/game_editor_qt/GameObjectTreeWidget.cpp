//
// Created by mgrus on 18.04.2025.
//

#include "GameObjectTreeWidget.h"


GameObjectTreeWidget::GameObjectTreeWidget(QWidget* parent) : QTreeWidget(parent) {
    this->setHeaderHidden(true); // no header if you want that minimal look
    this->setStyleSheet("background-color: #111; color: white;");

    // Add some test nodes
    auto root = new QTreeWidgetItem(this, QStringList("Player"));
    new QTreeWidgetItem(root, QStringList("Camera"));
    new QTreeWidgetItem(root, QStringList("Collider"));

    auto enemy = new QTreeWidgetItem(this, QStringList("Enemy"));
    new QTreeWidgetItem(enemy, QStringList("AI"));
    new QTreeWidgetItem(enemy, QStringList("Health"));

    this->expandAll(); // for now

}
