#ifndef MESSENGERSERVER_H
#define MESSENGERSERVER_H

#include <QMap>
#include <QObject>
#include <QSqlDatabase>
#include <QTcpServer>
#include <QTcpSocket>

namespace Messenger {

class MessengerServer : public QObject {
    Q_OBJECT
  public:
    static MessengerServer &instance();

    bool start(quint16 port = 45454);
    void stop();
    bool isListening() const;

  private slots:
    void onNewConnection();
    void onClientDisconnected();
    void onReadyRead();

  private:
    MessengerServer(QObject *parent = nullptr);
    ~MessengerServer();

    bool initDatabase();
    void handlePacket(QTcpSocket *socket, const QJsonObject &json);
    void sendPacket(QTcpSocket *socket, const QJsonObject &json);
    void broadcast(const QJsonObject &json, QTcpSocket *exclude = nullptr);
    void broadcastUserList();

    QTcpServer                  *m_tcpServer = nullptr;
    QMap<QTcpSocket *, QByteArray> m_clientBuffers;
    QMap<QTcpSocket *, QString>   m_socketToUser;
    QMap<QString, QTcpSocket *>   m_userToSocket;
    QSqlDatabase                 m_db;
};

} // namespace Messenger

#endif // MESSENGERSERVER_H
