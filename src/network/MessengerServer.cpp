#include "MessengerServer.h"
#include "NetworkProtocol.h"

#include <QDataStream>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlError>
#include <QSqlQuery>

Messenger::MessengerServer &Messenger::MessengerServer::instance() {
    static MessengerServer inst;
    return inst;
}

Messenger::MessengerServer::MessengerServer(QObject *parent)
    : QObject(parent)
    , m_tcpServer(new QTcpServer(this))
{
    connect(m_tcpServer, &QTcpServer::newConnection, this, &MessengerServer::onNewConnection);
}

Messenger::MessengerServer::~MessengerServer() {
    stop();
}

bool Messenger::MessengerServer::initDatabase() {
    if (QSqlDatabase::contains("ServerConnection")) {
        m_db = QSqlDatabase::database("ServerConnection");
        return true;
    }

    m_db = QSqlDatabase::addDatabase("QSQLITE", "ServerConnection");
    m_db.setDatabaseName("server_data.db");
    if (!m_db.open()) {
        qDebug() << "Server DB error:" << m_db.lastError().text();
        return false;
    }

    QSqlQuery q(m_db);
    q.exec("CREATE TABLE IF NOT EXISTS users ("
           "username TEXT PRIMARY KEY, "
           "password TEXT, "
           "display_name TEXT)");

    return true;
}

bool Messenger::MessengerServer::start(quint16 port) {
    if (m_tcpServer->isListening()) {
        return true;
    }

    initDatabase();

    if (!m_tcpServer->listen(QHostAddress::Any, port)) {
        qDebug() << "MessengerServer could not listen on port" << port << ":" << m_tcpServer->errorString();
        return false;
    }

    qDebug() << "MessengerServer started on port" << port;
    return true;
}

void Messenger::MessengerServer::stop() {
    if (m_tcpServer->isListening()) {
        m_tcpServer->close();
    }
}

bool Messenger::MessengerServer::isListening() const {
    return m_tcpServer && m_tcpServer->isListening();
}

void Messenger::MessengerServer::onNewConnection() {
    while (m_tcpServer->hasPendingConnections()) {
        QTcpSocket *socket = m_tcpServer->nextPendingConnection();
        m_clientBuffers[socket] = QByteArray();

        connect(socket, &QTcpSocket::readyRead, this, &MessengerServer::onReadyRead);
        connect(socket, &QTcpSocket::disconnected, this, &MessengerServer::onClientDisconnected);
        qDebug() << "Server: New client connected from" << socket->peerAddress().toString();
    }
}

void Messenger::MessengerServer::onClientDisconnected() {
    auto *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket)
        return;

    QString username = m_socketToUser.value(socket);
    if (!username.isEmpty()) {
        m_userToSocket.remove(username);
        m_socketToUser.remove(socket);
        qDebug() << "Server: User" << username << "disconnected";
        broadcastUserList();
    }

    m_clientBuffers.remove(socket);
    socket->deleteLater();
}

void Messenger::MessengerServer::onReadyRead() {
    auto *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket)
        return;

    QByteArray &buffer = m_clientBuffers[socket];
    buffer.append(socket->readAll());

    while (buffer.size() >= static_cast<int>(sizeof(quint32))) {
        QDataStream stream(buffer);
        stream.setByteOrder(QDataStream::BigEndian);
        quint32 packetSize = 0;
        stream >> packetSize;

        if (buffer.size() < static_cast<int>(sizeof(quint32) + packetSize)) {
            break; // Ждем прихода остатка пакета
        }

        QByteArray jsonData = buffer.mid(sizeof(quint32), packetSize);
        buffer.remove(0, sizeof(quint32) + packetSize);

        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        if (doc.isObject()) {
            handlePacket(socket, doc.object());
        }
    }
}

void Messenger::MessengerServer::sendPacket(QTcpSocket *socket, const QJsonObject &json) {
    if (!socket || socket->state() != QAbstractSocket::ConnectedState)
        return;

    QByteArray jsonData = QJsonDocument(json).toJson(QJsonDocument::Compact);
    quint32 packetSize = static_cast<quint32>(jsonData.size());

    QByteArray packet;
    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << packetSize;
    packet.append(jsonData);

    socket->write(packet);
}

void Messenger::MessengerServer::broadcast(const QJsonObject &json, QTcpSocket *exclude) {
    for (auto it = m_socketToUser.begin(); it != m_socketToUser.end(); ++it) {
        if (it.key() != exclude) {
            sendPacket(it.key(), json);
        }
    }
}

void Messenger::MessengerServer::handlePacket(QTcpSocket *socket, const QJsonObject &json) {
    QString action = json.value("action").toString();

    if (action == Action::Register) {
        QString username = json.value("username").toString().trimmed().toLower();
        QString password = json.value("password").toString();
        QString displayName = json.value("display_name").toString().trimmed();

        if (username.isEmpty() || password.isEmpty()) {
            QJsonObject res;
            res["action"] = "register_result";
            res["success"] = false;
            res["message"] = "Логин и пароль не могут быть пустыми";
            sendPacket(socket, res);
            return;
        }

        if (displayName.isEmpty()) {
            displayName = username;
        }

        QSqlQuery check(m_db);
        check.prepare("SELECT username FROM users WHERE username = :u");
        check.bindValue(":u", username);
        check.exec();

        if (check.next()) {
            QJsonObject res;
            res["action"] = "register_result";
            res["success"] = false;
            res["message"] = "Пользователь с таким логином уже существует";
            sendPacket(socket, res);
            return;
        }

        QSqlQuery insert(m_db);
        insert.prepare("INSERT INTO users (username, password, display_name) VALUES (:u, :p, :dn)");
        insert.bindValue(":u", username);
        insert.bindValue(":p", password);
        insert.bindValue(":dn", displayName);
        if (insert.exec()) {
            QJsonObject res;
            res["action"] = "register_result";
            res["success"] = true;
            res["message"] = "Регистрация прошла успешно!";
            sendPacket(socket, res);
            broadcastUserList();
        } else {
            QJsonObject res;
            res["action"] = "register_result";
            res["success"] = false;
            res["message"] = "Ошибка базы данных при регистрации";
            sendPacket(socket, res);
        }
    } else if (action == Action::Login) {
        QString username = json.value("username").toString().trimmed().toLower();
        QString password = json.value("password").toString();

        QSqlQuery q(m_db);
        q.prepare("SELECT password, display_name FROM users WHERE username = :u");
        q.bindValue(":u", username);
        q.exec();

        if (q.next()) {
            QString realPass = q.value("password").toString();
            QString dispName = q.value("display_name").toString();

            if (realPass == password) {
                m_socketToUser[socket] = username;
                m_userToSocket[username] = socket;

                QJsonObject res;
                res["action"] = "login_result";
                res["success"] = true;
                res["username"] = username;
                res["display_name"] = dispName;
                sendPacket(socket, res);

                broadcastUserList();
                return;
            }
        }

        QJsonObject res;
        res["action"] = "login_result";
        res["success"] = false;
        res["message"] = "Неверный логин или пароль";
        sendPacket(socket, res);
    } else if (action == Action::GetUsers) {
        broadcastUserList();
    } else if (action == Action::Message) {
        QString recipient = json.value("to").toString().trimmed().toLower();
        QTcpSocket *recSocket = m_userToSocket.value(recipient, nullptr);
        if (recSocket && recSocket->state() == QAbstractSocket::ConnectedState) {
            sendPacket(recSocket, json);
        }
    } else if (action == Action::Typing) {
        QString recipient = json.value("to").toString().trimmed().toLower();
        QTcpSocket *recSocket = m_userToSocket.value(recipient, nullptr);
        if (recSocket && recSocket->state() == QAbstractSocket::ConnectedState) {
            sendPacket(recSocket, json);
        }
    }
}

void Messenger::MessengerServer::broadcastUserList() {
    QJsonArray usersArray;
    QSqlQuery q("SELECT username, display_name FROM users", m_db);
    while (q.next()) {
        QString uname = q.value("username").toString();
        QString dname = q.value("display_name").toString();
        bool isOnline = m_userToSocket.contains(uname);

        QJsonObject u;
        u["username"] = uname;
        u["display_name"] = dname;
        u["is_online"] = isOnline;
        usersArray.append(u);
    }

    QJsonObject packet;
    packet["action"] = Action::UserList;
    packet["users"] = usersArray;

    for (auto it = m_socketToUser.begin(); it != m_socketToUser.end(); ++it) {
        sendPacket(it.key(), packet);
    }
}
