#include "MessageModel.h"

Messenger::MessageModel::MessageModel(QObject *parent)
    : QAbstractListModel(parent)
{}

int Messenger::MessageModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return 0;
    return m_messages.size();
}

QVariant Messenger::MessageModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();
    if (index.row() < 0 || index.row() >= m_messages.size())
        return QVariant();

    const auto &msg = m_messages[index.row()];

    switch (role) {
    case Qt::DisplayRole:
        return msg.text;
    case Messenger::Roles::IdRole:
        return msg.id;
    case Messenger::Roles::IsOutgoingRole:
        return msg.isOutgoing;
    case Messenger::Roles::TimestampRole:
        return msg.timestamp;
    case Messenger::Roles::FilePathRole:
        return msg.filePath;
    case Messenger::Roles::ReplyToIdRole:
        return msg.replyToId;
    case Messenger::Roles::ReplyTextRole:
        return msg.replyText;
    case Messenger::Roles::IsEditedRole:
        return msg.isEdited;
    case Messenger::Roles::IsReadRole:
        return msg.isRead;
    default:
        return QVariant();
    }
}

void Messenger::MessageModel::setMessages(const QList<Message> &messages) {
    beginResetModel();
    m_messages = messages;
    endResetModel();
}

void Messenger::MessageModel::addMessage(const Message &message) {
    beginInsertRows(QModelIndex(), m_messages.size(), m_messages.size());
    m_messages.append(message);
    endInsertRows();
}

bool Messenger::MessageModel::updateMessage(int id, const QString &newText) {
    for (int i = 0; i < m_messages.size(); ++i) {
        if (m_messages[i].id == id) {
            m_messages[i].text = newText;
            m_messages[i].isEdited = true;
            QModelIndex idx = index(i);
            emit dataChanged(idx, idx, {Qt::DisplayRole, Messenger::Roles::IsEditedRole});
            return true;
        }
    }
    return false;
}

bool Messenger::MessageModel::removeMessage(int id) {
    for (int i = 0; i < m_messages.size(); ++i) {
        if (m_messages[i].id == id) {
            beginRemoveRows(QModelIndex(), i, i);
            m_messages.removeAt(i);
            endRemoveRows();
            return true;
        }
    }
    return false;
}

Messenger::Message Messenger::MessageModel::messageById(int id) const {
    for (const auto &msg : m_messages) {
        if (msg.id == id)
            return msg;
    }
    return Message();
}

Messenger::Message Messenger::MessageModel::messageAt(int row) const {
    if (row >= 0 && row < m_messages.size())
        return m_messages[row];
    return Message();
}

Messenger::Message Messenger::MessageModel::lastOutgoingMessage() const {
    for (int i = m_messages.size() - 1; i >= 0; --i) {
        if (m_messages[i].isOutgoing) {
            return m_messages[i];
        }
    }
    return Message();
}

void Messenger::MessageModel::clear() {
    beginResetModel();
    m_messages.clear();
    endResetModel();
}
