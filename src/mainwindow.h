#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

class QListWidget;
class QVBoxLayout;
class QTextEdit;
class QLineEdit;
class QPushButton;
class QLabel;

class MainWindow : public QWidget {
    Q_OBJECT
  public:
    explicit MainWindow(QWidget *parent = nullptr);

  private slots:
    void onSendBtnClicked();
    void onChatClicked();

  private:
    void chatSelectedHandle(const QString &chatTitle);

    QLabel      *m_chatTitle    = nullptr;
    QTextEdit   *m_messagesView = nullptr;
    QLineEdit   *m_messageInput = nullptr;
    QPushButton *m_sendButton   = nullptr;
    QListWidget *m_chatList     = nullptr;
};

#endif // MAINWINDOW_H
