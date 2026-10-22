#include "LoginDialog.h"
#include "network/MessengerServer.h"
#include "network/NetworkClient.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>

Messenger::LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Telegram — Вход в аккаунт");
    setFixedSize(380, 420);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    setupUI();

    // Запускаем локальный сервер, если он еще не запущен (первый экземпляр станет сервером)
    MessengerServer::instance().start(DefaultPort);

    // Подключаемся к клиенту
    auto &client = NetworkClient::instance();
    connect(&client, &NetworkClient::connected, this, &LoginDialog::onConnected);
    connect(&client, &NetworkClient::disconnected, this, &LoginDialog::onDisconnected);
    connect(&client, &NetworkClient::loginResult, this, &LoginDialog::onLoginResult);
    connect(&client, &NetworkClient::registerResult, this, &LoginDialog::onRegisterResult);

    client.connectToServer("127.0.0.1", DefaultPort);
}

void Messenger::LoginDialog::setupUI() {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(14);

    // Заголовок
    auto *titleLabel = new QLabel("Вход в Telegram", this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("color: #ffffff; background: transparent;");

    m_statusLabel = new QLabel("🟡 Подключение к серверу...", this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("color: #7f91a4; font-size: 11px; background: transparent;");

    // Переключатель Вход / Регистрация
    auto *tabLayout = new QHBoxLayout();
    tabLayout->setSpacing(8);

    m_tabLoginBtn = new QPushButton("Вход", this);
    m_tabRegBtn = new QPushButton("Регистрация", this);

    m_tabLoginBtn->setCursor(Qt::PointingHandCursor);
    m_tabRegBtn->setCursor(Qt::PointingHandCursor);

    auto updateTabStyles = [this](bool isLogin) {
        if (isLogin) {
            m_tabLoginBtn->setStyleSheet("background-color: #2b5278; color: #ffffff; border-radius: 6px; padding: 6px 12px;");
            m_tabRegBtn->setStyleSheet("background-color: transparent; color: #7f91a4; border: 1px solid #242f3d; border-radius: 6px; padding: 6px 12px;");
        } else {
            m_tabLoginBtn->setStyleSheet("background-color: transparent; color: #7f91a4; border: 1px solid #242f3d; border-radius: 6px; padding: 6px 12px;");
            m_tabRegBtn->setStyleSheet("background-color: #2b5278; color: #ffffff; border-radius: 6px; padding: 6px 12px;");
        }
    };
    updateTabStyles(true);

    tabLayout->addWidget(m_tabLoginBtn);
    tabLayout->addWidget(m_tabRegBtn);

    m_stackedWidget = new QStackedWidget(this);

    // 1. Страница Входа
    auto *loginPage = new QWidget();
    auto *loginLayout = new QVBoxLayout(loginPage);
    loginLayout->setContentsMargins(0, 8, 0, 0);
    loginLayout->setSpacing(10);

    m_loginUserEdit = new QLineEdit(loginPage);
    m_loginUserEdit->setPlaceholderText("Логин (например, alexey или irina)");

    m_loginPassEdit = new QLineEdit(loginPage);
    m_loginPassEdit->setPlaceholderText("Пароль");
    m_loginPassEdit->setEchoMode(QLineEdit::Password);

    m_loginBtn = new QPushButton("Войти в мессенджер", loginPage);
    m_loginBtn->setCursor(Qt::PointingHandCursor);
    m_loginBtn->setStyleSheet("background-color: #2b5278; color: #ffffff; padding: 10px; border-radius: 8px; font-weight: bold;");

    connect(m_loginBtn, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(m_loginPassEdit, &QLineEdit::returnPressed, this, &LoginDialog::onLoginClicked);

    loginLayout->addWidget(new QLabel("Логин:", loginPage));
    loginLayout->addWidget(m_loginUserEdit);
    loginLayout->addWidget(new QLabel("Пароль:", loginPage));
    loginLayout->addWidget(m_loginPassEdit);
    loginLayout->addSpacing(6);
    loginLayout->addWidget(m_loginBtn);
    loginLayout->addStretch();

    // 2. Страница Регистрации
    auto *regPage = new QWidget();
    auto *regLayout = new QVBoxLayout(regPage);
    regLayout->setContentsMargins(0, 8, 0, 0);
    regLayout->setSpacing(10);

    m_regUserEdit = new QLineEdit(regPage);
    m_regUserEdit->setPlaceholderText("Логин (латиницей, например irina)");

    m_regNameEdit = new QLineEdit(regPage);
    m_regNameEdit->setPlaceholderText("Отображаемое имя (например, Ирина)");

    m_regPassEdit = new QLineEdit(regPage);
    m_regPassEdit->setPlaceholderText("Пароль");
    m_regPassEdit->setEchoMode(QLineEdit::Password);

    m_regBtn = new QPushButton("Создать аккаунт", regPage);
    m_regBtn->setCursor(Qt::PointingHandCursor);
    m_regBtn->setStyleSheet("background-color: #4fae63; color: #ffffff; padding: 10px; border-radius: 8px; font-weight: bold;");

    connect(m_regBtn, &QPushButton::clicked, this, &LoginDialog::onRegisterClicked);
    connect(m_regPassEdit, &QLineEdit::returnPressed, this, &LoginDialog::onRegisterClicked);

    regLayout->addWidget(new QLabel("Логин аккаунта:", regPage));
    regLayout->addWidget(m_regUserEdit);
    regLayout->addWidget(new QLabel("Имя контакта:", regPage));
    regLayout->addWidget(m_regNameEdit);
    regLayout->addWidget(new QLabel("Пароль:", regPage));
    regLayout->addWidget(m_regPassEdit);
    regLayout->addSpacing(6);
    regLayout->addWidget(m_regBtn);
    regLayout->addStretch();

    m_stackedWidget->addWidget(loginPage);
    m_stackedWidget->addWidget(regPage);

    connect(m_tabLoginBtn, &QPushButton::clicked, this, [this, updateTabStyles]() {
        m_stackedWidget->setCurrentIndex(0);
        updateTabStyles(true);
    });
    connect(m_tabRegBtn, &QPushButton::clicked, this, [this, updateTabStyles]() {
        m_stackedWidget->setCurrentIndex(1);
        updateTabStyles(false);
    });

    // Быстрый запуск для тестирования 2 окон
    auto *demoLayout = new QHBoxLayout();
    demoLayout->setSpacing(8);

    auto *demo1Btn = new QPushButton("🚀 Вход как Алексей", this);
    auto *demo2Btn = new QPushButton("🚀 Вход как Ирина", this);
    demo1Btn->setCursor(Qt::PointingHandCursor);
    demo2Btn->setCursor(Qt::PointingHandCursor);
    demo1Btn->setStyleSheet("background-color: #242f3d; color: #65aadd; font-size: 11px; padding: 6px; border-radius: 6px;");
    demo2Btn->setStyleSheet("background-color: #242f3d; color: #ee7aae; font-size: 11px; padding: 6px; border-radius: 6px;");

    connect(demo1Btn, &QPushButton::clicked, this, &LoginDialog::onDemoAlexeyClicked);
    connect(demo2Btn, &QPushButton::clicked, this, &LoginDialog::onDemoIrinaClicked);

    demoLayout->addWidget(demo1Btn);
    demoLayout->addWidget(demo2Btn);

    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(m_statusLabel);
    mainLayout->addLayout(tabLayout);
    mainLayout->addWidget(m_stackedWidget);
    mainLayout->addSpacing(4);
    mainLayout->addWidget(new QLabel("Быстрый вход для тестирования двух чатов:", this));
    mainLayout->addLayout(demoLayout);
}

void Messenger::LoginDialog::onConnected() {
    m_statusLabel->setText("🟢 Подключено к серверу (127.0.0.1:45454)");
    m_statusLabel->setStyleSheet("color: #4fae63; font-size: 11px; background: transparent;");
}

void Messenger::LoginDialog::onDisconnected() {
    m_statusLabel->setText("🔴 Нет подключения к серверу");
    m_statusLabel->setStyleSheet("color: #e17076; font-size: 11px; background: transparent;");
}

void Messenger::LoginDialog::onLoginClicked() {
    QString user = m_loginUserEdit->text().trimmed();
    QString pass = m_loginPassEdit->text();

    if (user.isEmpty() || pass.isEmpty()) {
        QMessageBox::warning(this, "Внимание", "Заполните логин и пароль.");
        return;
    }

    m_statusLabel->setText("Авторизация...");
    NetworkClient::instance().loginAccount(user, pass);
}

void Messenger::LoginDialog::onRegisterClicked() {
    QString user = m_regUserEdit->text().trimmed();
    QString name = m_regNameEdit->text().trimmed();
    QString pass = m_regPassEdit->text();

    if (user.isEmpty() || pass.isEmpty()) {
        QMessageBox::warning(this, "Внимание", "Логин и пароль обязательны.");
        return;
    }

    if (name.isEmpty()) {
        name = user;
    }

    m_statusLabel->setText("Регистрация...");
    NetworkClient::instance().registerAccount(user, pass, name);
}

void Messenger::LoginDialog::onDemoAlexeyClicked() {
    // Автоматическая регистрация/вход Алексея
    m_statusLabel->setText("Авто-вход: Алексей...");
    NetworkClient::instance().registerAccount("alexey", "123", "Алексей");
    QTimer::singleShot(300, this, []() {
        NetworkClient::instance().loginAccount("alexey", "123");
    });
}

void Messenger::LoginDialog::onDemoIrinaClicked() {
    // Автоматическая регистрация/вход Ирины
    m_statusLabel->setText("Авто-вход: Ирина...");
    NetworkClient::instance().registerAccount("irina", "123", "Ирина");
    QTimer::singleShot(300, this, []() {
        NetworkClient::instance().loginAccount("irina", "123");
    });
}

void Messenger::LoginDialog::onRegisterResult(bool success, const QString &message) {
    if (success) {
        m_statusLabel->setText("Регистрация успешна! Вход...");
        // Если регистрировались через форму — логинимся сразу
        QString user = m_regUserEdit->text().trimmed();
        QString pass = m_regPassEdit->text();
        if (!user.isEmpty()) {
            NetworkClient::instance().loginAccount(user, pass);
        }
    } else {
        m_statusLabel->setText(message);
        m_statusLabel->setStyleSheet("color: #e17076; font-size: 11px;");
    }
}

void Messenger::LoginDialog::onLoginResult(bool success, const QString &message,
                                           const QString &username, const QString &displayName) {
    if (success) {
        m_username = username;
        m_displayName = displayName;
        accept();
    } else {
        m_statusLabel->setText(message);
        m_statusLabel->setStyleSheet("color: #e17076; font-size: 11px;");
        QMessageBox::warning(this, "Ошибка входа", message);
    }
}
