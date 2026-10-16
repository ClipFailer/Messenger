#include "mainwindow.h"
#include "chatmodel.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QSplitter>
#include <QTextEdit>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) : QWidget{parent} {
    setWindowTitle("TelegramClone");
    resize(800, 600);

    // Начальные данные.
    m_chatModel = new ChatModel(this);
    m_chatModel->addChat("Ирина");
    m_chatModel->addChat("Витя");
    m_chatModel->addChat("Алексей");
    m_chatModel->addMessage("Ирина", "Привет!");
    m_chatModel->addMessage("Витя", "Занят?");
    m_chatModel->addMessage("Алексей", "Я скоро буду.");

    QSplitter *splitter = new QSplitter(this);

    // Левая панель.
    m_chatList = new QListWidget();
    updateChats();

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
    if (!m_chatModel || m_currentChat.isEmpty())
        return;

    QString message = m_messageInput->text().trimmed();

    if (message.isEmpty())
        return;

    m_messagesView->append(message);
    m_chatModel->addMessage(m_currentChat, message);
    m_messageInput->clear();
}

void MainWindow::onChatClicked(QListWidgetItem *item, QListWidgetItem * /*previous*/) {
    if (!m_chatModel || !item)
        return;

    m_chatTitle->setText(item->text());
    m_currentChat = item->text();

    m_messagesView->clear();
    for (const QString &msg : m_chatModel->messages(m_currentChat)) {
        m_messagesView->append(msg);
    }
}

void MainWindow::updateChats() {
    m_chatList->clear();

    for (const QString &chatName : m_chatModel->chats()) {
        m_chatList->addItem(chatName);
    }
}
