#include "ChatModel.h"
#include "DatabaseManager.h"
#include <algorithm>

Messenger::ChatModel::ChatModel(QObject *parent)
    : QAbstractListModel(parent)
{}

int Messenger::ChatModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return 0;
    return m_chats.size();
}

QVariant Messenger::ChatModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_chats.size())
        return QVariant();

    const ChatInfo &chat = m_chats[index.row()];

    switch (role) {
    case Qt::DisplayRole:
        return chat.name;
    case Messenger::ChatRoles::idRole:
        return chat.id;
    case Messenger::ChatRoles::IsPinnedRole:
        return chat.isPinned;
    case Messenger::ChatRoles::ChatTypeRole:
        return chat.type;
    case Messenger::ChatRoles::UnreadCountRole:
        return chat.unreadCount;
    case Messenger::ChatRoles::AvatarColorRole:
        return chat.avatarColor;
    case Messenger::ChatRoles::LastMessageRole: {
        const auto &msgs = m_messages[chat.id];
        if (msgs.isEmpty())
            return QString("Нет сообщений");
        const auto &lastMsg = msgs.last();
        if (!lastMsg.filePath.isEmpty() && lastMsg.text.isEmpty())
            return QString("📷 Фотография");
        return lastMsg.text;
    }
    case Messenger::ChatRoles::LastTimeRole: {
        const auto &msgs = m_messages[chat.id];
        if (msgs.isEmpty())
            return QString();
        return msgs.last().timestamp;
    }
    default:
        return QVariant();
    }
}

QList<Messenger::ChatInfo> Messenger::ChatModel::chats() const {
    return m_chats;
}

Messenger::ChatInfo Messenger::ChatModel::chatInfo(int chatId) const {
    for (const auto &chat : m_chats) {
        if (chat.id == chatId)
            return chat;
    }
    return ChatInfo();
}

QList<Messenger::Message> Messenger::ChatModel::messages(int chatId) const {
    return m_messages.value(chatId);
}

bool Messenger::ChatModel::hasChat(int chatId) const {
    for (const auto &chat : m_chats) {
        if (chat.id == chatId)
            return true;
    }
    return false;
}

int Messenger::ChatModel::nextChatId() const {
    int maxId = 0;
    for (const auto &chat : m_chats) {
        if (chat.id > maxId)
            maxId = chat.id;
    }
    return maxId + 1;
}

int Messenger::ChatModel::findRowByChatId(int chatId) const {
    for (int i = 0; i < m_chats.size(); ++i) {
        if (m_chats[i].id == chatId)
            return i;
    }
    return -1;
}

void Messenger::ChatModel::addChat(int chatId, const QString &chatName, const QString &type,
                                   bool isPinned, const QString &avatarColor) {
    if (hasChat(chatId))
        return;

    ChatInfo newChat;
    newChat.id = chatId;
    newChat.name = chatName;
    newChat.type = type;
    newChat.isPinned = isPinned;
    newChat.avatarColor = avatarColor;
    newChat.unreadCount = 0;

    int insertRow = 0;
    if (!isPinned) {
        // Вставляем после закрепленных
        while (insertRow < m_chats.size() && m_chats[insertRow].isPinned) {
            insertRow++;
        }
    }

    beginInsertRows(QModelIndex(), insertRow, insertRow);
    m_chats.insert(insertRow, newChat);
    m_messages[chatId] = QList<Message>();
    endInsertRows();

    DatabaseManager::instance().saveChat(newChat);
}

void Messenger::ChatModel::deleteChat(int row) {
    if (row < 0 || row >= m_chats.size())
        return;

    const int chatId = m_chats[row].id;

    beginRemoveRows(QModelIndex(), row, row);
    m_chats.removeAt(row);
    m_messages.remove(chatId);
    endRemoveRows();

    DatabaseManager::instance().removeChat(chatId);
}

void Messenger::ChatModel::deleteChatById(int chatId) {
    int row = findRowByChatId(chatId);
    if (row != -1) {
        deleteChat(row);
    }
}

void Messenger::ChatModel::clearChatHistory(int chatId) {
    if (!m_messages.contains(chatId))
        return;

    m_messages[chatId].clear();
    DatabaseManager::instance().clearChatMessages(chatId);

    int row = findRowByChatId(chatId);
    if (row != -1) {
        QModelIndex idx = index(row);
        emit dataChanged(idx, idx, {ChatRoles::LastMessageRole, ChatRoles::LastTimeRole});
    }
}

void Messenger::ChatModel::togglePinChat(int chatId) {
    int row = findRowByChatId(chatId);
    if (row == -1)
        return;

    m_chats[row].isPinned = !m_chats[row].isPinned;
    DatabaseManager::instance().setChatPinned(chatId, m_chats[row].isPinned);

    sortChats();
}

void Messenger::ChatModel::addMessage(int chatId, Message &message) {
    DatabaseManager::instance().saveMessage(chatId, message);

    m_messages[chatId].append(message);

    int currentRow = findRowByChatId(chatId);
    if (currentRow == -1)
        return;

    // Перемещаем чат вверх списка (сохраняя приоритет закрепленных чатов)
    bool isPinned = m_chats[currentRow].isPinned;
    int targetRow = isPinned ? 0 : 0;
    if (!isPinned) {
        while (targetRow < m_chats.size() && m_chats[targetRow].isPinned) {
            targetRow++;
        }
    }

    if (currentRow != targetRow && currentRow > targetRow) {
        beginMoveRows(QModelIndex(), currentRow, currentRow, QModelIndex(), targetRow);
        ChatInfo chat = m_chats.takeAt(currentRow);
        m_chats.insert(targetRow, chat);
        endMoveRows();
        currentRow = targetRow;
    }

    QModelIndex idx = index(currentRow);
    emit dataChanged(idx, idx, {ChatRoles::LastMessageRole, ChatRoles::LastTimeRole, ChatRoles::UnreadCountRole});
}

void Messenger::ChatModel::updateMessage(int chatId, int messageId, const QString &newText) {
    if (!m_messages.contains(chatId))
        return;

    for (auto &msg : m_messages[chatId]) {
        if (msg.id == messageId) {
            msg.text = newText;
            msg.isEdited = true;
            break;
        }
    }

    DatabaseManager::instance().updateMessage(messageId, newText);

    int row = findRowByChatId(chatId);
    if (row != -1) {
        QModelIndex idx = index(row);
        emit dataChanged(idx, idx, {ChatRoles::LastMessageRole});
    }
}

void Messenger::ChatModel::deleteMessage(int chatId, int messageId) {
    if (!m_messages.contains(chatId))
        return;

    for (int i = 0; i < m_messages[chatId].size(); ++i) {
        if (m_messages[chatId][i].id == messageId) {
            m_messages[chatId].removeAt(i);
            break;
        }
    }

    DatabaseManager::instance().removeMessage(messageId);

    int row = findRowByChatId(chatId);
    if (row != -1) {
        QModelIndex idx = index(row);
        emit dataChanged(idx, idx, {ChatRoles::LastMessageRole, ChatRoles::LastTimeRole});
    }
}

void Messenger::ChatModel::setUnreadCount(int chatId, int count) {
    int row = findRowByChatId(chatId);
    if (row != -1) {
        m_chats[row].unreadCount = count;
        QModelIndex idx = index(row);
        emit dataChanged(idx, idx, {ChatRoles::UnreadCountRole});
    }
}

void Messenger::ChatModel::markChatAsRead(int chatId) {
    setUnreadCount(chatId, 0);
}

void Messenger::ChatModel::loadFromDB() {
    beginResetModel();
    DatabaseManager::instance().loadChats(m_chats, m_messages);
    endResetModel();
}

void Messenger::ChatModel::sortChats() {
    beginResetModel();
    std::stable_sort(m_chats.begin(), m_chats.end(), [](const ChatInfo &a, const ChatInfo &b) {
        if (a.isPinned != b.isPinned)
            return a.isPinned > b.isPinned;
        return a.id < b.id;
    });
    endResetModel();
}
