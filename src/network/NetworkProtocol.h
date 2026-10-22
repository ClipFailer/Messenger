#ifndef NETWORKPROTOCOL_H
#define NETWORKPROTOCOL_H

#include <QString>
#include <QList>

namespace Messenger {

constexpr quint16 DefaultPort = 45454;

struct RemoteUser {
    QString username;
    QString displayName;
    bool isOnline = false;
};

// Имена действий в JSON-пакетах
namespace Action {
    inline const QString Register      = "register";
    inline const QString Login         = "login";
    inline const QString GetUsers      = "get_users";
    inline const QString UserList      = "user_list";
    inline const QString Message       = "message";
    inline const QString Typing        = "typing";
    inline const QString StatusChange  = "status_change";
    inline const QString Error         = "error";
    inline const QString Success       = "success";
}

} // namespace Messenger

#endif // NETWORKPROTOCOL_H
