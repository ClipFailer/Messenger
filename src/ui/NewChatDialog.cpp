#include "NewChatDialog.h"
#include "network/NetworkClient.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QVBoxLayout>

Messenger::NewChatDialog::NewChatDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Новый чат");
    setFixedSize(380, 380);

    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(10);

    auto *onlineLabel = new QLabel("Пользователи в сети (кликните для выбора):", this);
    m_usersList = new QListWidget(this);
    m_usersList->setFixedHeight(100);
    m_usersList->setStyleSheet("QListWidget { background-color: #242f3d; border: 1px solid #242f3d; border-radius: 6px; } "
                               "QListWidget::item { padding: 4px; } "
                               "QListWidget::item:selected { background-color: #2b5278; }");

    connect(m_usersList, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        QString uname = item->data(Qt::UserRole).toString();
        m_nameEdit->setText(uname);
        m_typeCombo->setCurrentIndex(0); // Личный чат
    });

    auto *nameLabel = new QLabel("Имя контакта или название:", this);
    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText("Логин (например: irina, alexey) или имя");

    auto *typeLabel = new QLabel("Тип чата:", this);
    m_typeCombo = new QComboBox(this);
    m_typeCombo->addItem("Личный контакт", "user");
    m_typeCombo->addItem("Группа", "group");
    m_typeCombo->addItem("Канал", "channel");
    m_typeCombo->addItem("Чат-бот", "bot");

    auto *msgLabel = new QLabel("Первое сообщение (необязательно):", this);
    m_initialMsgEdit = new QLineEdit(this);
    m_initialMsgEdit->setPlaceholderText("Привет!");

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        if (!m_nameEdit->text().trimmed().isEmpty()) {
            accept();
        }
    });
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    layout->addWidget(onlineLabel);
    layout->addWidget(m_usersList);
    layout->addWidget(nameLabel);
    layout->addWidget(m_nameEdit);
    layout->addWidget(typeLabel);
    layout->addWidget(m_typeCombo);
    layout->addWidget(msgLabel);
    layout->addWidget(m_initialMsgEdit);
    layout->addStretch();
    layout->addWidget(buttonBox);

    // Запрашиваем актуальный список пользователей сети
    connect(&NetworkClient::instance(), &NetworkClient::userListReceived, this, &NewChatDialog::onUserListReceived);
    NetworkClient::instance().requestUserList();

    m_nameEdit->setFocus();
}

void Messenger::NewChatDialog::onUserListReceived(const QList<RemoteUser> &users) {
    m_usersList->clear();
    QString myUser = NetworkClient::instance().currentUsername();
    for (const auto &u : users) {
        if (u.username == myUser) continue;
        QString status = u.isOnline ? "🟢 [В сети]" : "⚪ [Офлайн]";
        auto *item = new QListWidgetItem(QString("%1 %2 (@%3)").arg(status, u.displayName, u.username), m_usersList);
        item->setData(Qt::UserRole, u.username);
    }
}

QString Messenger::NewChatDialog::chatName() const {
    return m_nameEdit->text().trimmed();
}

QString Messenger::NewChatDialog::chatType() const {
    return m_typeCombo->currentData().toString();
}

QString Messenger::NewChatDialog::initialMessage() const {
    return m_initialMsgEdit->text().trimmed();
}
