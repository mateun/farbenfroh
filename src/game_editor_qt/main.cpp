//
// Created by mgrus on 16.04.2025.
//

#include <QApplication>
#include <QMainWindow>

int main(int argc, char *argv[]) {
    qputenv("QT_DEBUG_PLUGINS", "1");
    QApplication app(argc, argv);

    QMainWindow mainWindow;
    mainWindow.setWindowTitle("My Qt Editor");
    mainWindow.resize(800, 600);
    mainWindow.show();

    return app.exec();
}
