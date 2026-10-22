#ifndef NEWCHATDIALOG_H
#define NEWCHATDIALOG_H

#include "network/NetworkProtocol.h"
#include <QDialog>
#include <QList>

class QLineEdit;
class QComboBox;
class QListWidget;

namespace Messenger {

class NewChatDialog : public QDialog {
    Q_OBJECT
  public:
    explicit NewChatDialog(QWidget *parent = nullptr);

    QString chatName() const;
    QString chatType() const;
    QString initialMessage() const;

  private slots:
    void onUserListReceived(const QList<RemoteUser> &users);

  private:
    QLineEdit   *m_nameEdit = nullptr;
    QComboBox   *m_typeCombo = nullptr;
    QLineEdit   *m_initialMsgEdit = nullptr;
    QListWidget *m_usersList = nullptr;
};

} // namespace Messenger

#endif // NEWCHATDIALOG_H
