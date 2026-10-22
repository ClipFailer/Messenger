#include "DatabaseManager.h"
#include "ChatModel.h"
#include "MessageModel.h"
#include <QDateTime>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

Messenger::DatabaseManager &Messenger::DatabaseManager::instance() {
    static DatabaseManager inst;
    return inst;
}

bool Messenger::DatabaseManager::init(const QString &dbName) {
    if (m_db.isOpen()) {
        m_db.close();
    }

    if (QSqlDatabase::contains("ClientConnection")) {
        m_db = QSqlDatabase::database("ClientConnection");
    } else {
        m_db = QSqlDatabase::addDatabase("QSQLITE", "ClientConnection");
    }
    m_db.setDatabaseName(dbName);

    if (!m_db.open()) {
        qDebug() << "DB Error: " << m_db.lastError().text();
        return false;
    }

    QSqlQuery query(m_db);
    // Создаем таблицу чатов
    query.exec("CREATE TABLE IF NOT EXISTS chats ("
               "id INTEGER PRIMARY KEY, "
               "name TEXT, "
               "type TEXT DEFAULT 'user', "
               "is_pinned INTEGER DEFAULT 0, "
               "avatar_color TEXT DEFAULT '')");

    // Создаем таблицу сообщений
    query.exec("CREATE TABLE IF NOT EXISTS messages ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "chat_id INTEGER, "
               "text TEXT, "
               "is_outgoing INTEGER, "
               "timestamp TEXT DEFAULT '', "
               "file_path TEXT DEFAULT '', "
               "reply_to_id INTEGER DEFAULT 0, "
               "reply_text TEXT DEFAULT '', "
               "is_edited INTEGER DEFAULT 0)");

    // Безопасное добавление колонок на случай обновления существующей БД
    query.exec("ALTER TABLE chats ADD COLUMN type TEXT DEFAULT 'user'");
    query.exec("ALTER TABLE chats ADD COLUMN is_pinned INTEGER DEFAULT 0");
    query.exec("ALTER TABLE chats ADD COLUMN avatar_color TEXT DEFAULT ''");

    query.exec("ALTER TABLE messages ADD COLUMN timestamp TEXT DEFAULT ''");
    query.exec("ALTER TABLE messages ADD COLUMN file_path TEXT DEFAULT ''");
    query.exec("ALTER TABLE messages ADD COLUMN reply_to_id INTEGER DEFAULT 0");
    query.exec("ALTER TABLE messages ADD COLUMN reply_text TEXT DEFAULT ''");
    query.exec("ALTER TABLE messages ADD COLUMN is_edited INTEGER DEFAULT 0");

    seedInitialDataIfEmpty();

    qDebug() << "Database successfully initialized:" << dbName;
    return true;
}

void Messenger::DatabaseManager::seedInitialDataIfEmpty() {
    QSqlQuery checkQuery("SELECT COUNT(*) FROM chats", m_db);
    if (checkQuery.next() && checkQuery.value(0).toInt() == 0) {
        // Создаем стартовые чаты в духе Telegram
        ChatInfo savedChat;
        savedChat.id = 1;
        savedChat.name = "Избранное";
        savedChat.type = "saved";
        savedChat.isPinned = true;
        savedChat.avatarColor = "#2AABEE";
        saveChat(savedChat);

        Message savedMsg("Здесь можно хранить полезные заметки, ссылки и файлы как в облаке Telegram.", true,
                         QDateTime::currentDateTime().addSecs(-3600).toString("HH:mm"));
        saveMessage(savedChat.id, savedMsg);

        ChatInfo tgChat;
        tgChat.id = 2;
        tgChat.name = "Служба Telegram";
        tgChat.type = "channel";
        tgChat.isPinned = true;
        tgChat.avatarColor = "#0088cc";
        saveChat(tgChat);

        Message tgMsg("Добро пожаловать в Telegram Clone! Теперь чат поддерживает работу по сети, регистрацию и переписку в реальном времени.", false,
                      QDateTime::currentDateTime().addSecs(-1800).toString("HH:mm"));
        saveMessage(tgChat.id, tgMsg);

        ChatInfo botChat;
        botChat.id = 3;
        botChat.name = "Чат-бот Ассистент";
        botChat.type = "bot";
        botChat.isPinned = false;
        botChat.avatarColor = "#4fae63";
        saveChat(botChat);

        Message botMsg("Привет! Я встроенный бот. Напишите мне сообщение или команду /help, чтобы увидеть список функций.", false,
                       QDateTime::currentDateTime().addSecs(-600).toString("HH:mm"));
        saveMessage(botChat.id, botMsg);
    }
}

void Messenger::DatabaseManager::saveChat(const ChatInfo &chat) {
    QSqlQuery query(m_db);
    query.prepare("INSERT OR REPLACE INTO chats (id, name, type, is_pinned, avatar_color) "
                  "VALUES (:id, :name, :type, :is_pinned, :avatar_color)");
    query.bindValue(":id", chat.id);
    query.bindValue(":name", chat.name);
    query.bindValue(":type", chat.type);
    query.bindValue(":is_pinned", chat.isPinned ? 1 : 0);
    query.bindValue(":avatar_color", chat.avatarColor);

    if (!query.exec()) {
        qDebug() << "Failed to save chat:" << query.lastError().text();
    }
}

void Messenger::DatabaseManager::removeChat(int id) {
    clearChatMessages(id);

    QSqlQuery query(m_db);
    query.prepare("DELETE FROM chats WHERE id = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        qDebug() << "Failed to remove chat:" << query.lastError().text();
    }
}

void Messenger::DatabaseManager::clearChatMessages(int chatId) {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM messages WHERE chat_id = :chat_id");
    query.bindValue(":chat_id", chatId);
    if (!query.exec()) {
        qDebug() << "Failed to clear chat messages:" << query.lastError().text();
    }
}

void Messenger::DatabaseManager::setChatPinned(int id, bool pinned) {
    QSqlQuery query(m_db);
    query.prepare("UPDATE chats SET is_pinned = :pinned WHERE id = :id");
    query.bindValue(":pinned", pinned ? 1 : 0);
    query.bindValue(":id", id);
    if (!query.exec()) {
        qDebug() << "Failed to update chat pinned state:" << query.lastError().text();
    }
}

void Messenger::DatabaseManager::saveMessage(int chatId, Message &message) {
    message.chatId = chatId;
    if (message.timestamp.isEmpty()) {
        message.timestamp = QDateTime::currentDateTime().toString("HH:mm");
    }

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO messages (chat_id, text, is_outgoing, timestamp, file_path, reply_to_id, reply_text, is_edited) "
                  "VALUES (:chat_id, :text, :is_outgoing, :timestamp, :file_path, :reply_to_id, :reply_text, :is_edited)");
    query.bindValue(":chat_id", chatId);
    query.bindValue(":text", message.text);
    query.bindValue(":is_outgoing", message.isOutgoing ? 1 : 0);
    query.bindValue(":timestamp", message.timestamp);
    query.bindValue(":file_path", message.filePath);
    query.bindValue(":reply_to_id", message.replyToId);
    query.bindValue(":reply_text", message.replyText);
    query.bindValue(":is_edited", message.isEdited ? 1 : 0);

    if (query.exec()) {
        message.id = query.lastInsertId().toInt();
    } else {
        qDebug() << "Failed to save message:" << query.lastError().text();
    }
}

void Messenger::DatabaseManager::updateMessage(int messageId, const QString &newText) {
    QSqlQuery query(m_db);
    query.prepare("UPDATE messages SET text = :text, is_edited = 1 WHERE id = :id");
    query.bindValue(":text", newText);
    query.bindValue(":id", messageId);
    if (!query.exec()) {
        qDebug() << "Failed to update message:" << query.lastError().text();
    }
}

void Messenger::DatabaseManager::removeMessage(int messageId) {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM messages WHERE id = :id");
    query.bindValue(":id", messageId);
    if (!query.exec()) {
        qDebug() << "Failed to remove message:" << query.lastError().text();
    }
}

void Messenger::DatabaseManager::loadChats(QList<ChatInfo> &outChats,
                                           QMap<int, QList<Message>> &outMessages) {
    outChats.clear();
    outMessages.clear();

    QSqlQuery query("SELECT id, name, type, is_pinned, avatar_color FROM chats ORDER BY is_pinned DESC, id ASC", m_db);
    while (query.next()) {
        ChatInfo chat;
        chat.id = query.value("id").toInt();
        chat.name = query.value("name").toString();
        chat.type = query.value("type").toString();
        if (chat.type.isEmpty()) chat.type = "user";
        chat.isPinned = query.value("is_pinned").toInt() == 1;
        chat.avatarColor = query.value("avatar_color").toString();
        chat.unreadCount = 0;

        outChats.append(chat);
        outMessages[chat.id] = QList<Message>();
    }

    QSqlQuery msgQuery("SELECT id, chat_id, text, is_outgoing, timestamp, file_path, reply_to_id, reply_text, is_edited "
                       "FROM messages ORDER BY id ASC", m_db);
    while (msgQuery.next()) {
        Message msg;
        msg.id = msgQuery.value("id").toInt();
        msg.chatId = msgQuery.value("chat_id").toInt();
        msg.text = msgQuery.value("text").toString();
        msg.isOutgoing = msgQuery.value("is_outgoing").toInt() == 1;
        msg.timestamp = msgQuery.value("timestamp").toString();
        if (msg.timestamp.isEmpty()) {
            msg.timestamp = "12:00";
        }
        msg.filePath = msgQuery.value("file_path").toString();
        msg.replyToId = msgQuery.value("reply_to_id").toInt();
        msg.replyText = msgQuery.value("reply_text").toString();
        msg.isEdited = msgQuery.value("is_edited").toInt() == 1;

        if (outMessages.contains(msg.chatId)) {
            outMessages[msg.chatId].append(msg);
        }
    }
}
