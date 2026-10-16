#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

class QListWidget;
class QVBoxLayout;
class QListView;
class QLineEdit;
class QPushButton;
class QLabel;
class QListWidgetItem;

namespace Messenger {

class ChatModel;
class MessageModel;

class MainWindow : public QWidget {
    Q_OBJECT
  public:
    explicit MainWindow(QWidget *parent = nullptr);

  private slots:
    void onSendBtnClicked();
    void onChatClicked(QListWidgetItem *item, QListWidgetItem *previous);

  private:
    // Обновляет список чатов
    void updateChats();

    QLabel      *m_chatTitle    = nullptr;
    QListView   *m_messagesView = nullptr;
    QLineEdit   *m_messageInput = nullptr;
    QPushButton *m_sendButton   = nullptr;
    QListWidget *m_chatList     = nullptr;

    ChatModel *m_chatModel = nullptr;
    QString    m_currentChat;

    MessageModel *m_messageModel = nullptr;
};
} // namespace Messenger

#endif // MAINWINDOW_H
