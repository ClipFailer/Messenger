#include "ChatModel.h"
#include "MessageModel.h"

Messenger::ChatModel::ChatModel(QObject *parent) : QAbstractListModel{parent} {}

int Messenger::ChatModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return 0;

    return m_chats.size();
}

QVariant Messenger::ChatModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_chats.size())
        return QVariant();

    const ChatInfo &chat = m_chats[index.row()]; // Достаем инфу о чате

    if (role == Qt::DisplayRole)
        return chat.name;

    if (role == Messenger::ChatRoles::idRole)
        return chat.id;

    if (role == Messenger::ChatRoles::LastMessageRole) {
        // Берем историю по ID чата, а не по индексу строки!
        if (m_messages[chat.id].isEmpty())
            return "Нет сообщений";
        return m_messages[chat.id].last().text;
    }

    return QVariant();
}

void Messenger::ChatModel::addChat(const int chatId, const QString &chatName) {
    if (hasChat(chatId))
        return;

    beginInsertRows(QModelIndex(), m_chats.size(), m_chats.size());

    m_chats.append(ChatInfo{chatId, chatName});
    m_messages[chatId] = QList<Message>();

    endInsertRows();
}

void Messenger::ChatModel::addMessage(const int chatId, const Message &message) {
    m_messages[chatId].append(message);
}

QList<Messenger::ChatInfo> Messenger::ChatModel::chats() const {
    return m_chats;
}

QList<Messenger::Message> Messenger::ChatModel::messages(const int chatId) const {
    return m_messages[chatId];
}

bool Messenger::ChatModel::hasChat(const int chatId) const {
    return m_messages.contains(chatId);
}