#include "ui/LoginDialog.h"
#include "ui/MainWindow.h"
#include <QApplication>

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    Messenger::LoginDialog loginDlg;
    if (loginDlg.exec() == QDialog::Accepted) {
        Messenger::MainWindow w(loginDlg.username(), loginDlg.displayName());
        w.show();
        return app.exec();
    }

    return 0;
}
