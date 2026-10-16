#include "mainwindow.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSplitter>
#include <QTextEdit>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) : QWidget{parent} {
    setWindowTitle("TelegramClone");
    resize(800, 600);

    // Начальные данные.
    m_messagesByChat["Анна"] << "Привет!" << "Как дела?";
    m_messagesByChat["Борис"] << "Созвон в 15:00";
    m_messagesByChat["Виктор"] << "Файлы отправил";

    QSplitter *splitter = new QSplitter(this);

    // Левая панель (чаты).
    m_chatList = new QListWidget();
    m_chatList->addItem("Анна");
    m_chatList->addItem("Борис");
    m_chatList->addItem("Виктор");

    // Правая панель (чат).
    QWidget     *chatWidget = new QWidget();
    QVBoxLayout *chatLayout = new QVBoxLayout(chatWidget);

    m_chatTitle    = new QLabel("Выберите чат", chatWidget);
    m_messagesView = new QTextEdit(chatWidget);
    m_messagesView->setReadOnly(true);

    m_messageInput                 = new QLineEdit(chatWidget);
    m_sendButton                   = new QPushButton("Отправить", chatWidget);
    QHBoxLayout *sendMessageLayout = new QHBoxLayout();
    sendMessageLayout->addWidget(m_messageInput);
    sendMessageLayout->addWidget(m_sendButton);

    chatLayout->addWidget(m_chatTitle);
    chatLayout->addWidget(m_messagesView);
    chatLayout->addLayout(sendMessageLayout);

    // разделитель
    splitter->addWidget(m_chatList);
    splitter->addWidget(chatWidget);
    splitter->setSizes({250, 550});

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(splitter);

    // Бинд кнопки отправить.
    connect(m_sendButton, &QPushButton::clicked, this, &MainWindow::onSendBtnClicked);
    connect(m_messageInput, &QLineEdit::returnPressed, this, &MainWindow::onSendBtnClicked);

    connect(m_chatList, &QListWidget::currentItemChanged, this, &MainWindow::onChatClicked);
}

void MainWindow::onSendBtnClicked() {
    if (m_currentChat.isEmpty())
        return;

    QString message = m_messageInput->text().trimmed();

    if (message.isEmpty())
        return;

    m_messagesView->append(message);
    m_messagesByChat[m_currentChat].append(message);
    m_messageInput->clear();
}

void MainWindow::onChatClicked(QListWidgetItem *item, QListWidgetItem * /*previous*/) {
    if (!item)
        return;

    m_chatTitle->setText(item->text());
    m_currentChat = item->text();

    m_messagesView->clear();
    const QStringList &messages = m_messagesByChat.value(m_currentChat);
    for (const QString &msg : messages) {
        m_messagesView->append(msg);
    }
}
