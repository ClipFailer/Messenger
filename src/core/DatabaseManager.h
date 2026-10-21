#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>

namespace Messenger {

class Message;
struct ChatInfo;

/**
 * @brief Класс базы данных мессенджера.
 *
 * Синглтон класс, описывающий базу данных мессенджера.
 * Подключает бд SQlite, создает таблицы (чаты, сообщения), если они не созданы.
 */
class DatabaseManager {
  public:
    /// Создает синглтон, если он еще не создан и возвращает его.
    static DatabaseManager &instance();

    /// Инициализирует бд.
    bool init();

    /// Сохраняет чат.
    void saveChat(const int id, const QString &name);

    /// Сохраняет сообещние.
    void saveMessage(const int chatId, const Message &message);

    /// Удаляет чат.
    void removeChat(const int id);

    // Загружает и возвращает все чаты и сообщения.
    void loadChats(QList<ChatInfo> &outChats, QMap<int, QList<Message>> &outMessages);

  private:
    DatabaseManager()  = default;
    ~DatabaseManager() = default;

    DatabaseManager(const DatabaseManager &)            = delete;
    DatabaseManager &operator=(const DatabaseManager &) = delete;

    QSqlDatabase m_db;
};
} // namespace Messenger

#endif // DATABASEMANAGER_H
