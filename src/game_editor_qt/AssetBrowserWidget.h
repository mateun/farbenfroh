//
// Created by mgrus on 17.04.2025.
//

#ifndef ASSETBROWSERWIDGET_H
#define ASSETBROWSERWIDGET_H
#include <QWidget>


class AssetBrowserWidget : public QWidget {

public:
    AssetBrowserWidget(QWidget *parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent* event) override;

};



#endif //ASSETBROWSERWIDGET_H
