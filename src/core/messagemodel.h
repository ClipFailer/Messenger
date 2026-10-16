#ifndef MESSAGEMODEL_H
#define MESSAGEMODEL_H

#include <QAbstractListModel> // ← ListModel, не ItemModel
#include <QStringList>

namespace Messenger {
enum Roles { IsOutgoingRole = Qt::UserRole + 1 };

struct Message {
    Message(const QString &text, bool isOutgoing = true) : text(text), isOutgoing(isOutgoing) {}

    QString text;
    bool    isOutgoing;
};

class MessageModel : public QAbstractListModel {
    Q_OBJECT
  public:
    explicit MessageModel(QObject *parent = nullptr);

    //===== Overrides =====
    int      rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void setMessages(const QList<Message> &messages);
    void addMessage(const Message &message);
    void clear();

  private:
    QList<Message> m_messages;
};
} // namespace Messenger

#endif // MESSAGEMODEL_H
