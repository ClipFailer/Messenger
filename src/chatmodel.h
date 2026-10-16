#ifndef CHATMODEL_H
#define CHATMODEL_H

#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>

class ChatModel : public QObject {
    Q_OBJECT
  public:
    explicit ChatModel(QObject *parent = nullptr);

    void addChat(const QString &chatName);
    void addMessage(const QString &chatName, const QString &message);

    QStringList chats() const;
    QStringList messages(const QString &chatName) const;
    bool        hasChat(const QString &chatName) const;

  private:
    QMap<QString, QStringList> m_messagesByChat;
};

#endif // CHATMODEL_H
