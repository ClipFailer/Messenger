#include "MainWindow.h"
#include "core/ChatModel.h"
#include "core/MessageModel.h"
#include "ui/MessageDelegate.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>

Messenger::MainWindow::MainWindow(QWidget *parent) : QWidget{parent} {
    // Настройки окна.
    setWindowTitle("TelegramClone");
    resize(800, 600);

    // Начальные данные.
    m_chatModel = new ChatModel(this);
    m_chatModel->addChat("Ирина");
    m_chatModel->addChat("Витя");
    m_chatModel->addChat("Алексей");
    m_chatModel->addMessage("Ирина", Messenger::Message("Привет!", false));
    m_chatModel->addMessage("Витя", Messenger::Message("Занят?", false));
    m_chatModel->addMessage("Алексей", Messenger::Message("Скоро будешь?", false));

    QSplitter *splitter = new QSplitter(this);

    // Левая панель - список чатов.
    m_chatList = new QListWidget();
    updateChats();

    // Правая панель (чат).
    QWidget     *chatWidget = new QWidget();
    QVBoxLayout *chatLayout = new QVBoxLayout(chatWidget);

    m_chatTitle = new QLabel("Выберите чат", chatWidget);

    // Создаем View, Model чата и связывем их.
    m_messagesView = new QListView(chatWidget);
    m_messageModel = new MessageModel(this);
    m_messagesView->setModel(m_messageModel);
    m_messagesView->setItemDelegate(new MessageDelegate(this));
    m_messagesView->setSelectionMode(QAbstractItemView::NoSelection);
    m_messagesView->setFocusPolicy(Qt::NoFocus);
    m_messagesView->setFrameShape(QFrame::NoFrame);

    // Чат - область ввода сообщения.
    m_messageInput                 = new QLineEdit(chatWidget);
    m_sendButton                   = new QPushButton("Отправить", chatWidget);
    QHBoxLayout *sendMessageLayout = new QHBoxLayout();
    sendMessageLayout->addWidget(m_messageInput);
    sendMessageLayout->addWidget(m_sendButton);

    // Лейаут чата.
    chatLayout->addWidget(m_chatTitle);
    chatLayout->addWidget(m_messagesView);
    chatLayout->addLayout(sendMessageLayout);

    // Разделитель списка чатов (левая сторона) и активного чата (правая сторона).
    splitter->addWidget(m_chatList);
    splitter->addWidget(chatWidget);
    splitter->setSizes({250, 550});

    // Главный лэйаут
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(splitter);

    // Бинд кнопки отправить.
    connect(m_sendButton, &QPushButton::clicked, this, &MainWindow::onSendBtnClicked);
    connect(m_messageInput, &QLineEdit::returnPressed, this, &MainWindow::onSendBtnClicked);

    // Бинд смены часта.
    connect(m_chatList, &QListWidget::currentItemChanged, this, &MainWindow::onChatClicked);
}

void Messenger::MainWindow::onSendBtnClicked() {
    if (!m_chatModel || m_currentChat.isEmpty())
        return;

    QString messageText = m_messageInput->text().trimmed();
    if (messageText.isEmpty())
        return;

    const Messenger::Message message(messageText);

    m_messageModel->addMessage(message);
    m_chatModel->addMessage(m_currentChat, message);
    m_messageInput->clear();
}

void Messenger::MainWindow::onChatClicked(QListWidgetItem *item, QListWidgetItem * /*previous*/) {
    if (!m_chatModel || !item)
        return;

    m_chatTitle->setText(item->text());
    m_currentChat = item->text();
    m_messageModel->setMessages(m_chatModel->messages(m_currentChat));
}

void Messenger::MainWindow::updateChats() {
    m_chatList->clear();

    for (const QString &chatName : m_chatModel->chats()) {
        m_chatList->addItem(chatName);
    }
}
