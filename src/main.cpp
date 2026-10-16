#include "ui/MainWindow.h"
#include <QApplication>

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    Messenger::MainWindow w;
    w.show();

    return app.exec();
}