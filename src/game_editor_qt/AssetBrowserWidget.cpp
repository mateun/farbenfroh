//
// Created by mgrus on 17.04.2025.
//

#include "AssetBrowserWidget.h"
#include <QFile>
#include <QDropEvent>
#include <QMimeData>

AssetBrowserWidget::AssetBrowserWidget(QWidget *parent) : QWidget(parent) {
    setAcceptDrops(true);
}

void AssetBrowserWidget::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) {
        const auto urls = event->mimeData()->urls();
        for (const QUrl& url : urls) {
            if (url.toLocalFile().endsWith(".glb", Qt::CaseInsensitive)) {
                event->acceptProposedAction();
                return;
            }
        }
    }
    event->ignore();
}

void AssetBrowserWidget::dropEvent(QDropEvent* event) {
    const QList<QUrl> urls = event->mimeData()->urls();
    for (const QUrl& url : urls) {
        QString path = url.toLocalFile();
        if (path.endsWith(".glb", Qt::CaseInsensitive)) {
            QFile file(path);
            if (file.open(QIODevice::ReadOnly)) {
                QByteArray glbData = file.readAll();
                file.close();
                qDebug() << "GLB file dropped, size:" << glbData.size();

                // 🔥 Your code to forward this binary data into your engine
                // maybe emit a signal: emit glbDropped(glbData, path);
            }
        }
    }
}