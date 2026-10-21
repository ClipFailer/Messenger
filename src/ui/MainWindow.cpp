#include "MainWindow.h"
#include "core/ChatModel.h"
#include "core/MessageModel.h"
#include "ui/MessageDelegate.h"
#include "ui/chatdelegate.h"
#include <QAction>
#include <QDebug>
#include <QFile>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>

Messenger::MainWindow::MainWindow(QWidget *parent) : QWidget{parent} {
    // Подключение стилей.
    QFile styleFile(":/style.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QString::fromUtf8(styleFile.readAll());
        setStyleSheet(styleSheet);

        qDebug() << "Styles file has successfully loaded.";
    } else {
        qDebug() << "Failed to load styles file.";
    }
    // Настройки окна.
    setWindowTitle("TelegramClone");
    resize(800, 600);

    // Начальные данные.
    m_chatModel = new ChatModel(this);
    m_chatModel->addChat(0, "Ирина");
    m_chatModel->addChat(1, "Витя");
    m_chatModel->addChat(2, "Алексей");
    m_chatModel->addMessage(0, Messenger::Message("Привет!", false));
    m_chatModel->addMessage(1, Messenger::Message("Занят?", false));
    m_chatModel->addMessage(2, Messenger::Message("Скоро будешь?", false));

    QSplitter *splitter = new QSplitter(this);

    // Левая панель - список чатов.
    m_chatList = new QListView(this);
    m_chatList->setModel(m_chatModel);
    m_chatList->setItemDelegate(new ChatDelegate(this));
    m_chatList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_chatList, &QListView::customContextMenuRequested, this,
            &MainWindow::onChatContextMenu);

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
    connect(m_chatList, &QListView::clicked, this, &MainWindow::onChatClicked);
}

void Messenger::MainWindow::onSendBtnClicked() {
    if (!m_chatModel || m_currentChatId == -1)
        return;

    QString messageText = m_messageInput->text().trimmed();
    if (messageText.isEmpty())
        return;

    const Messenger::Message message(messageText);

    m_messageModel->addMessage(message);
    m_chatModel->addMessage(m_currentChatId, message);
    m_messageInput->clear();

    m_messagesView->scrollToBottom();
}

void Messenger::MainWindow::onChatClicked(const QModelIndex &index) {
    if (!m_chatModel)
        return;

    m_chatTitle->setText(index.data(Qt::DisplayRole).toString());
    m_currentChatId = index.data(Messenger::ChatRoles::idRole).toInt();
    m_messageModel->setMessages(m_chatModel->messages(m_currentChatId));

    m_messagesView->scrollToBottom();
}

void Messenger::MainWindow::onChatContextMenu(const QPoint &pos) {
    const QModelIndex clickedChatIndex = m_chatList->indexAt(pos);

    if (!clickedChatIndex.isValid())
        return;

    QMenu    menu(this);
    QAction *deleteAction = menu.addAction("Удалить чат");

    QAction *selectedAction = menu.exec(m_chatList->mapToGlobal(pos));
    if (selectedAction == deleteAction) {
        const int deletecChatId = clickedChatIndex.data(Messenger::ChatRoles::idRole).toInt();

        m_chatModel->deleteChat(clickedChatIndex.row());

        if (deletecChatId == m_currentChatId) {
            m_messageModel->clear();
            m_chatTitle->clear();
            m_currentChatId = -1;
        }
    }
}
