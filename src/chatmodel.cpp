#include "chatmodel.h"

ChatModel::ChatModel(QObject *parent) : QObject{parent} {}

void ChatModel::addChat(const QString &chatName) {
    if (m_messagesByChat.contains(chatName))
        return;

    m_messagesByChat[chatName] = QStringList();
}

void ChatModel::addMessage(const QString &chatName, const QString &message) {
    m_messagesByChat[chatName].append(message);
}

QStringList ChatModel::chats() const {
    return m_messagesByChat.keys();
}

QStringList ChatModel::messages(const QString &chatName) const {
    return m_messagesByChat.value(chatName);
}

bool ChatModel::hasChat(const QString &chatName) const {
    return m_messagesByChat.contains(chatName);
}