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

/**
 * @brief Главный класс окна.
 *
 * Хранит основную информацию и управляет процессом работы приложения.
 */
class MainWindow : public QWidget {
    Q_OBJECT
  public:
    explicit MainWindow(QWidget *parent = nullptr);

  private slots:
    /// Обрабатывает отправку сообщения.
    void onSendBtnClicked();

    /// Обрабатывает смену чата.
    void onChatClicked(const QModelIndex &index);

  private:
    /// Обновляет список чатов
    void updateChats();

    QLabel      *m_chatTitle    = nullptr; //< Заголовок чата.
    QListView   *m_messagesView = nullptr; //< Зона чата.
    QLineEdit   *m_messageInput = nullptr; //< Поле ввода сообщения.
    QPushButton *m_sendButton   = nullptr; //< Кнопка отправки сообщения.
    QListView   *m_chatList     = nullptr; //< Список чатов.

    ChatModel *m_chatModel     = nullptr;
    int        m_currentChatId = -1;

    MessageModel *m_messageModel = nullptr;
};
} // namespace Messenger

#endif // MAINWINDOW_H
