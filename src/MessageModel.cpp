#include "MessageModel.h"

MessageModel::MessageModel(QObject *parent) : QAbstractListModel(parent) {}

int MessageModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return 0;
    return m_messages.size();
}

QVariant MessageModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();
    if (index.row() < 0 || index.row() >= m_messages.size())
        return QVariant();
    if (role == Qt::DisplayRole)
        return m_messages[index.row()];
    return QVariant();
}

void MessageModel::setMessages(const QStringList &messages) {
    beginResetModel();
    m_messages = messages;
    endResetModel();
}

void MessageModel::addMessage(const QString &message) {
    beginInsertRows(QModelIndex(), m_messages.size(), m_messages.size());
    m_messages.append(message);
    endInsertRows();
}

void MessageModel::clear() {
    beginResetModel();
    m_messages.clear();
    endResetModel();
}