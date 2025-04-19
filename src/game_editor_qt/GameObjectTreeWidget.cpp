//
// Created by mgrus on 18.04.2025.
//

#include "GameObjectTreeWidget.h"

#include <QInputDialog>
#include <QMenu>


GameObjectTreeWidget::GameObjectTreeWidget(QWidget* parent) : QTreeWidget(parent) {

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QTreeWidget::customContextMenuRequested,
            this, &GameObjectTreeWidget::showContextMenu);

    this->setHeaderHidden(true); // no header if you want that minimal look
    this->setStyleSheet("background-color: #111; color: white;");
    connect(this, &QTreeWidget::itemSelectionChanged, this, &GameObjectTreeWidget::selectionChanged);

    // Add some test nodes
    auto root = setGenRoot();


}

void GameObjectTreeWidget::selectionChanged() {
    if (!selectedItems().isEmpty()) {
        QTreeWidgetItem* item = selectedItems().first();
        auto gameObject = item->data(0, Qt::UserRole).value<edqt::GameObject*>();
        if (gameObject) {
            emit gameObjectSelected(gameObject);
        }
    }

}

QTreeWidgetItem *GameObjectTreeWidget::setGenRoot() {
    auto root = new QTreeWidgetItem(this, QStringList("GameObjects"));
    return root;
}

void GameObjectTreeWidget::showContextMenu(const QPoint& pos) {
    QTreeWidgetItem* item = itemAt(pos);
    if (!item) return;

    QMenu menu(this);
    QAction* createAction = menu.addAction("New GameObject");
    QAction* deleteAction = menu.addAction("Delete GameObject");

    QAction* chosen = menu.exec(viewport()->mapToGlobal(pos));

    if (chosen == createAction) {
        // Call your new-folder logic
        qDebug() << "Creating folder under " << item;
        bool ok;
        QString name = QInputDialog::getText(
            this,
            "Create New GameObject",
            "Folder Name:",
            QLineEdit::Normal,
            "",
            &ok
        );

        if (ok && !name.isEmpty()) {

            auto ngo = new edqt::GameObject();
            ngo->name = name.toStdString();
            level_->gameObjects.push_back(ngo);
            auto newGob = new QTreeWidgetItem(item, QStringList(name));
            newGob->setData(0, Qt::UserRole, QVariant::fromValue(ngo));
            expandItem(item);
        }

    }
}

/**
 * This is the function which we learn about a new level that has been loaded/created.
 *
 * @param level The new current level of the editor.
 */
void GameObjectTreeWidget::setLevel(edqt::Level *level) {
    level_ = level;
    clear();
    auto root = setGenRoot();
    for (auto go: level->gameObjects) {
        new QTreeWidgetItem(root, QStringList(QString::fromStdString(go->name)));
    }


    this->expandAll(); // for now
}
