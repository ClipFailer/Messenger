#ifndef CHATMODEL_H
#define CHATMODEL_H

#include "MessageModel.h"
#include <QAbstractListModel>
#include <QMap>
#include <QString>
#include <QList>

namespace Messenger {

/// Структура, описывающая базовые данные чата для левой панели.
struct ChatInfo {
    int     id = 0;                  //< Уникальный идентификатор чата.
    QString name;                    //< Название чата / имя контакта.
    QString type = "user";           //< Тип: "user", "group", "channel", "bot", "saved".
    bool    isPinned = false;        //< Закреплен ли чат вверху списка.
    QString avatarColor;             //< Цвет аватара (если пустой, генерируется по имени).
    int     unreadCount = 0;         //< Количество непрочитанных сообщений.
};

/// Перечисление для получения данных от модели.
enum ChatRoles {
    idRole          = Qt::UserRole + 1, //< ID чата.
    LastMessageRole = Qt::UserRole + 2, //< Последнее сообщение чата.
    LastTimeRole    = Qt::UserRole + 3, //< Время последнего сообщения.
    IsPinnedRole    = Qt::UserRole + 4, //< Флаг закрепления чата.
    ChatTypeRole    = Qt::UserRole + 5, //< Тип чата ("user", "bot", etc.).
    UnreadCountRole = Qt::UserRole + 6, //< Количество непрочитанных сообщений.
    AvatarColorRole = Qt::UserRole + 7  //< Цвет аватара.
};

/**
 * @brief Модель данных списка чатов в стиле Telegram.
 *
 * Хранит упорядоченный список чатов и историю сообщений.
 */
class ChatModel : public QAbstractListModel {
    Q_OBJECT

  public:
    explicit ChatModel(QObject *parent = nullptr);

    //====== Перегрузки ======
    int      rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    //======= Геттеры =======
    QList<ChatInfo> chats() const;
    ChatInfo        chatInfo(int chatId) const;
    QList<Message>  messages(int chatId) const;
    bool            hasChat(int chatId) const;
    int             nextChatId() const;
    int             findRowByChatId(int chatId) const;

    //======= Мутации =======
    void addChat(int chatId, const QString &chatName, const QString &type = "user",
                 bool isPinned = false, const QString &avatarColor = QString());
    void deleteChat(int row);
    void deleteChatById(int chatId);
    void clearChatHistory(int chatId);
    void togglePinChat(int chatId);

    void addMessage(int chatId, Message &message);
    void updateMessage(int chatId, int messageId, const QString &newText);
    void deleteMessage(int chatId, int messageId);

    void setUnreadCount(int chatId, int count);
    void markChatAsRead(int chatId);

    void loadFromDB();

  private:
    void sortChats();

    QList<ChatInfo>           m_chats;    //< Список чатов.
    QMap<int, QList<Message>> m_messages; //< Сообщения чатов по ID.
};

} // namespace Messenger

#endif // CHATMODEL_H
