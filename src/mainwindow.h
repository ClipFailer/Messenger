#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMap>
#include <QStringList>
#include <QWidget>

class QListWidget;
class QVBoxLayout;
class QTextEdit;
class QLineEdit;
class QPushButton;
class QLabel;
class QListWidgetItem;

class MainWindow : public QWidget {
    Q_OBJECT
  public:
    explicit MainWindow(QWidget *parent = nullptr);

  private slots:
    void onSendBtnClicked();
    void onChatClicked(QListWidgetItem *item, QListWidgetItem *previous);

  private:
    QLabel      *m_chatTitle    = nullptr;
    QTextEdit   *m_messagesView = nullptr;
    QLineEdit   *m_messageInput = nullptr;
    QPushButton *m_sendButton   = nullptr;
    QListWidget *m_chatList     = nullptr;

    QMap<QString, QStringList> m_messagesByChat;
    QString                    m_currentChat;
};

#endif // MAINWINDOW_H
