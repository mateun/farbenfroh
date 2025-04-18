//
// Created by mgrus on 17.04.2025.
//

#ifndef ASSETBROWSERWIDGET_H
#define ASSETBROWSERWIDGET_H
#include <qstandarditemmodel.h>
#include <qtreewidget.h>
#include <QWidget>


class QListView;
class QTreeWidget;
struct Project;

class AssetBrowserWidget : public QWidget {
    Q_OBJECT
public:
    AssetBrowserWidget(QWidget *parent = nullptr);

    void onAssetDoubleClicked(const QModelIndex &index);

    void refreshAssetListFor(const std::string& folderPath);

    void onFolderSelected(QTreeWidgetItem *item, int column);

    void showFolderContextMenu(const QPoint &pos);

    void setProject(Project* project);

    signals:
        void assetDoubleClicked(const QString& assetPath);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent* event) override;


private:
    Project * project_ = nullptr;
    QTreeWidget* folderTree_ = nullptr;
    QListView* assetList_ = nullptr;
    QStandardItemModel * asset_list_model_ = nullptr;

    std::string currentFolderPath_;
    QTreeWidgetItem * folder_tree_root_ = nullptr;
};



#endif //ASSETBROWSERWIDGET_H
