#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QList>
#include <QMap>
#include <QSqlDatabase>
#include <QString>

namespace Messenger {

struct Message;
struct ChatInfo;

/**
 * @brief Класс базы данных мессенджера (SQLite).
 *
 * Синглтон для управления локальной БД (чаты, сообщения, закрепления, вложения).
 */
class DatabaseManager {
  public:
    static DatabaseManager &instance();

    /// Инициализирует БД и обновляет схему при необходимости.
    bool init(const QString &dbName = "messenger.db");

    /// Сохраняет или обновляет чат.
    void saveChat(const ChatInfo &chat);

    /// Удаляет чат и все его сообщения.
    void removeChat(int id);

    /// Очищает историю сообщений чата.
    void clearChatMessages(int chatId);

    /// Обновляет флаг закрепления чата.
    void setChatPinned(int id, bool pinned);

    /// Сохраняет сообщение и заполняет присвоенный message.id.
    void saveMessage(int chatId, Message &message);

    /// Обновляет текст сообщения и флаг редактирования.
    void updateMessage(int messageId, const QString &newText);

    /// Удаляет конкретное сообщение.
    void removeMessage(int messageId);

    /// Загружает все чаты и историю сообщений из БД.
    void loadChats(QList<ChatInfo> &outChats, QMap<int, QList<Message>> &outMessages);

  private:
    DatabaseManager()  = default;
    ~DatabaseManager() = default;

    DatabaseManager(const DatabaseManager &)            = delete;
    DatabaseManager &operator=(const DatabaseManager &) = delete;

    void seedInitialDataIfEmpty();

    QSqlDatabase m_db;
};

} // namespace Messenger

#endif // DATABASEMANAGER_H
