#ifndef CHATMODEL_H
#define CHATMODEL_H

#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>

namespace Messenger {

struct Message;

class ChatModel : public QObject {
  Q_OBJECT public : explicit ChatModel(QObject *parent = nullptr);

    void addChat(const QString &chatName);
    void addMessage(const QString &chatName, const Message &message);

    QStringList    chats() const;
    QList<Message> messages(const QString &chatName) const;
    bool           hasChat(const QString &chatName) const;

  private:
    QMap<QString, QList<Message>> m_messagesByChat;
};
} // namespace Messenger

#endif // CHATMODEL_H
