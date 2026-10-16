#include "MessageModel.h"

Messenger::MessageModel::MessageModel(QObject *parent) : QAbstractListModel(parent) {}

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
    if (role == Qt::DisplayRole)
        return m_messages[index.row()].text;
    if (role == Messenger::Roles::IsOutgoingRole)
        return m_messages[index.row()].isOutgoing;
    return QVariant();
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

void Messenger::MessageModel::clear() {
    beginResetModel();
    m_messages.clear();
    endResetModel();
}