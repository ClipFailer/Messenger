#include "ChatModel.h"
#include "MessageModel.h"

Messenger::ChatModel::ChatModel(QObject *parent) : QObject{parent} {}

void Messenger::ChatModel::addChat(const QString &chatName) {
    if (m_messagesByChat.contains(chatName))
        return;

    m_messagesByChat[chatName] = QList<Message>();
}

void Messenger::ChatModel::addMessage(const QString &chatName, const Message &message) {
    m_messagesByChat[chatName].append(message);
}

QStringList Messenger::ChatModel::chats() const {
    return m_messagesByChat.keys();
}

QList<Messenger::Message> Messenger::ChatModel::messages(const QString &chatName) const {
    return m_messagesByChat.value(chatName);
}

bool Messenger::ChatModel::hasChat(const QString &chatName) const {
    return m_messagesByChat.contains(chatName);
}