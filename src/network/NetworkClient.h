#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include "NetworkProtocol.h"
#include <QByteArray>
#include <QObject>
#include <QTcpSocket>

namespace Messenger {

class NetworkClient : public QObject {
    Q_OBJECT
  public:
    static NetworkClient &instance();

    void connectToServer(const QString &host = "127.0.0.1", quint16 port = 45454);
    void disconnectFromServer();
    bool isConnected() const;

    QString currentUsername() const { return m_currentUsername; }
    QString currentDisplayName() const { return m_currentDisplayName; }

    void registerAccount(const QString &username, const QString &password, const QString &displayName);
    void loginAccount(const QString &username, const QString &password);
    void sendMessage(const QString &toUser, const QString &text, const QString &filePath = QString(),
                     int replyToId = 0, const QString &replyText = QString());
    void sendTyping(const QString &toUser, bool isTyping);
    void requestUserList();

  signals:
    void connected();
    void disconnected();
    void registerResult(bool success, const QString &message);
    void loginResult(bool success, const QString &message, const QString &username, const QString &displayName);
    void userListReceived(const QList<RemoteUser> &users);
    void messageReceived(const QString &fromUser, const QString &toUser, const QString &text,
                         const QString &timestamp, const QString &filePath, const QString &replyText);
    void typingReceived(const QString &fromUser, bool isTyping);

  private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();

  private:
    NetworkClient(QObject *parent = nullptr);
    ~NetworkClient() = default;

    void sendPacket(const QJsonObject &json);
    void handlePacket(const QJsonObject &json);

    QTcpSocket *m_socket = nullptr;
    QByteArray  m_buffer;
    QString     m_currentUsername;
    QString     m_currentDisplayName;
};

} // namespace Messenger

#endif // NETWORKCLIENT_H
