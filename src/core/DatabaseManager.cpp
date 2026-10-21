#include "DatabaseManager.h"
#include "core/ChatModel.h"
#include "core/MessageModel.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

Messenger::DatabaseManager &Messenger::DatabaseManager::instance() {
    static DatabaseManager instance;
    return instance;
}

bool Messenger::DatabaseManager::init() {

    // задаем драйвер бд.
    m_db = QSqlDatabase::addDatabase("QSQLITE");

    // Создаем или подключаем файл бд.
    m_db.setDatabaseName("messenger.db");

    if (!m_db.open()) {
        qDebug() << "DB Error: " << m_db.lastError().text();
        return false;
    }

    // Создаем таблицу чатов, если ее нет.
    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS chats ("
               "id INTEGER PRIMARY KEY, "
               "name TEXT)");

    // Создаем таблицу сообщений, если нет.
    query.exec("CREATE TABLE IF NOT EXISTS messages("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "chat_id INTEGER, "
               "text TEXT, "
               "is_outgoing INTEGER)");

    qDebug() << "DB has successfully loaded.";

    return true;
}

void Messenger::DatabaseManager::saveChat(const int id, const QString &name) {
    QSqlQuery query;
    query.prepare("INSERT OR IGNORE INTO chats (id, name) VALUES (:id, :name)");
    query.bindValue(":id", id);
    query.bindValue(":name", name);

    if (!query.exec()) {
        qDebug() << "Failed to save chat: " << query.lastError().text();
    }
}

void Messenger::DatabaseManager::saveMessage(const int chatId, const Message &message) {
    QSqlQuery query;
    query.prepare(
        "INSERT INTO messages (chat_id, text, is_outgoing) VALUES (:chat_id, :text, :is_outgoing)");
    query.bindValue(":chat_id", chatId);
    query.bindValue(":text", message.text);
    query.bindValue(":is_outgoing", message.isOutgoing ? 1 : 0);

    if (!query.exec()) {
        qDebug() << "Failed to save message: " << query.lastError().text();
    }
}

void Messenger::DatabaseManager::removeChat(const int id) {

    // Удаяем все сообщения чата.
    QSqlQuery query;
    query.prepare("DELETE FROM messages WHERE chat_id = :chat_id");
    query.bindValue(":chat_id", id);
    if (!query.exec()) {
        qDebug() << "Failed to remove chat: " << query.lastError().text();
    }

    // Удалям чат.
    query.prepare("DELETE FROM chats WHERE id = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        qDebug() << "Failed to remove chat: " << query.lastError().text();
    }
}

void Messenger::DatabaseManager::loadChats(QList<ChatInfo>           &outChats,
                                           QMap<int, QList<Message>> &outMessages) {
    QSqlQuery query;

    // Загружаем чаты.
    query.exec("SELECT id, name FROM chats");
    while (query.next()) {
        const int     id   = query.value("id").toInt();
        const QString name = query.value("name").toString();

        outChats.append(ChatInfo{id, name});
        outMessages[id] = QList<Message>();
    }

    // Загружаем сообщения.
    query.exec("SELECT chat_id, text, is_outgoing FROM messages");
    while (query.next()) {
        const int     chatId     = query.value("chat_id").toInt();
        const QString text       = query.value("text").toString();
        const bool    isOutgoing = query.value("is_outgoing").toInt() == 1;

        outMessages[chatId].append(Message{text, isOutgoing});
    }
}