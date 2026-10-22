#include "NetworkClient.h"
#include <QDataStream>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

Messenger::NetworkClient &Messenger::NetworkClient::instance() {
    static NetworkClient inst;
    return inst;
}

Messenger::NetworkClient::NetworkClient(QObject *parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
{
    connect(m_socket, &QTcpSocket::connected, this, &NetworkClient::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &NetworkClient::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &NetworkClient::onReadyRead);
}

void Messenger::NetworkClient::connectToServer(const QString &host, quint16 port) {
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        emit connected();
        return;
    }
    m_socket->abort();
    m_socket->connectToHost(host, port);
}

void Messenger::NetworkClient::disconnectFromServer() {
    m_socket->disconnectFromHost();
}

bool Messenger::NetworkClient::isConnected() const {
    return m_socket && m_socket->state() == QAbstractSocket::ConnectedState;
}

void Messenger::NetworkClient::onConnected() {
    qDebug() << "Client: Connected to server.";
    emit connected();
}

void Messenger::NetworkClient::onDisconnected() {
    qDebug() << "Client: Disconnected from server.";
    emit disconnected();
}

void Messenger::NetworkClient::onReadyRead() {
    m_buffer.append(m_socket->readAll());

    while (m_buffer.size() >= static_cast<int>(sizeof(quint32))) {
        QDataStream stream(m_buffer);
        stream.setByteOrder(QDataStream::BigEndian);
        quint32 packetSize = 0;
        stream >> packetSize;

        if (m_buffer.size() < static_cast<int>(sizeof(quint32) + packetSize)) {
            break;
        }

        QByteArray jsonData = m_buffer.mid(sizeof(quint32), packetSize);
        m_buffer.remove(0, sizeof(quint32) + packetSize);

        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        if (doc.isObject()) {
            handlePacket(doc.object());
        }
    }
}

void Messenger::NetworkClient::sendPacket(const QJsonObject &json) {
    if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "Cannot send packet: socket not connected!";
        return;
    }

    QByteArray jsonData = QJsonDocument(json).toJson(QJsonDocument::Compact);
    quint32 packetSize = static_cast<quint32>(jsonData.size());

    QByteArray packet;
    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << packetSize;
    packet.append(jsonData);

    m_socket->write(packet);
}

void Messenger::NetworkClient::registerAccount(const QString &username, const QString &password,
                                               const QString &displayName) {
    QJsonObject obj;
    obj["action"] = Action::Register;
    obj["username"] = username;
    obj["password"] = password;
    obj["display_name"] = displayName;
    sendPacket(obj);
}

void Messenger::NetworkClient::loginAccount(const QString &username, const QString &password) {
    QJsonObject obj;
    obj["action"] = Action::Login;
    obj["username"] = username;
    obj["password"] = password;
    sendPacket(obj);
}

void Messenger::NetworkClient::sendMessage(const QString &toUser, const QString &text,
                                           const QString &filePath, int replyToId,
                                           const QString &replyText) {
    Q_UNUSED(replyToId)
    QJsonObject obj;
    obj["action"] = Action::Message;
    obj["from"] = m_currentUsername;
    obj["to"] = toUser;
    obj["text"] = text;
    obj["timestamp"] = QDateTime::currentDateTime().toString("HH:mm");
    obj["reply_text"] = replyText;

    if (!filePath.isEmpty() && QFile::exists(filePath)) {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            obj["file_data"] = QString::fromLatin1(file.readAll().toBase64());
            obj["file_name"] = QFileInfo(filePath).fileName();
        }
    }

    sendPacket(obj);
}

void Messenger::NetworkClient::sendTyping(const QString &toUser, bool isTyping) {
    QJsonObject obj;
    obj["action"] = Action::Typing;
    obj["from"] = m_currentUsername;
    obj["to"] = toUser;
    obj["is_typing"] = isTyping;
    sendPacket(obj);
}

void Messenger::NetworkClient::requestUserList() {
    QJsonObject obj;
    obj["action"] = Action::GetUsers;
    sendPacket(obj);
}

void Messenger::NetworkClient::handlePacket(const QJsonObject &json) {
    QString action = json.value("action").toString();

    if (action == "register_result") {
        bool success = json.value("success").toBool();
        QString message = json.value("message").toString();
        emit registerResult(success, message);
    } else if (action == "login_result") {
        bool success = json.value("success").toBool();
        QString message = json.value("message").toString();
        if (success) {
            m_currentUsername = json.value("username").toString();
            m_currentDisplayName = json.value("display_name").toString();
        }
        emit loginResult(success, message, m_currentUsername, m_currentDisplayName);
    } else if (action == Action::UserList) {
        QList<RemoteUser> list;
        QJsonArray arr = json.value("users").toArray();
        for (const auto &val : arr) {
            QJsonObject uObj = val.toObject();
            RemoteUser u;
            u.username = uObj.value("username").toString();
            u.displayName = uObj.value("display_name").toString();
            u.isOnline = uObj.value("is_online").toBool();
            list.append(u);
        }
        emit userListReceived(list);
    } else if (action == Action::Message) {
        QString from = json.value("from").toString();
        QString to = json.value("to").toString();
        QString text = json.value("text").toString();
        QString timestamp = json.value("timestamp").toString();
        QString replyText = json.value("reply_text").toString();
        QString localFilePath;

        if (json.contains("file_data")) {
            QByteArray data = QByteArray::fromBase64(json.value("file_data").toString().toLatin1());
            QString fileName = json.value("file_name").toString("attachment.dat");

            QDir cacheDir("received_files");
            if (!cacheDir.exists()) {
                cacheDir.mkpath(".");
            }

            localFilePath = cacheDir.filePath(QString("%1_%2").arg(QDateTime::currentMSecsSinceEpoch()).arg(fileName));
            QFile out(localFilePath);
            if (out.open(QIODevice::WriteOnly)) {
                out.write(data);
                out.close();
            }
        }

        emit messageReceived(from, to, text, timestamp, localFilePath, replyText);
    } else if (action == Action::Typing) {
        QString from = json.value("from").toString();
        bool isTyping = json.value("is_typing").toBool();
        emit typingReceived(from, isTyping);
    }
}
