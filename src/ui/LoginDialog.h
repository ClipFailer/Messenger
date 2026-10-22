#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

class QLineEdit;
class QPushButton;
class QLabel;
class QStackedWidget;

namespace Messenger {

class LoginDialog : public QDialog {
    Q_OBJECT
  public:
    explicit LoginDialog(QWidget *parent = nullptr);

    QString username() const { return m_username; }
    QString displayName() const { return m_displayName; }

  private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void onDemoAlexeyClicked();
    void onDemoIrinaClicked();
    void onLoginResult(bool success, const QString &message, const QString &username, const QString &displayName);
    void onRegisterResult(bool success, const QString &message);
    void onConnected();
    void onDisconnected();

  private:
    void setupUI();

    QString m_username;
    QString m_displayName;

    QLabel *m_statusLabel = nullptr;

    // Поля входа
    QLineEdit *m_loginUserEdit = nullptr;
    QLineEdit *m_loginPassEdit = nullptr;
    QPushButton *m_loginBtn = nullptr;

    // Поля регистрации
    QLineEdit *m_regUserEdit = nullptr;
    QLineEdit *m_regNameEdit = nullptr;
    QLineEdit *m_regPassEdit = nullptr;
    QPushButton *m_regBtn = nullptr;

    QStackedWidget *m_stackedWidget = nullptr;
    QPushButton    *m_tabLoginBtn = nullptr;
    QPushButton    *m_tabRegBtn = nullptr;
};

} // namespace Messenger

#endif // LOGINDIALOG_H
