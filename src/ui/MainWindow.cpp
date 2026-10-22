#include "MainWindow.h"
#include "EmojiPickerPopup.h"
#include "NewChatDialog.h"
#include "chatdelegate.h"
#include "core/BotSimulator.h"
#include "core/DatabaseManager.h"
#include "network/NetworkClient.h"
#include "ui/MessageDelegate.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDateTime>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>

Messenger::MainWindow::MainWindow(const QString &username, const QString &displayName, QWidget *parent)
    : QWidget(parent)
    , m_currentUsername(username)
    , m_displayName(displayName)
{
    setObjectName("mainWindow");

    // Инициализируем БД для текущего профиля (чтобы у каждого пользователя была своя изолированная история)
    QString dbName = QString("messenger_%1.db").arg(m_currentUsername);
    Messenger::DatabaseManager::instance().init(dbName);

    // Загрузка стилей
    setupStyles();

    setWindowTitle(QString("Telegram — %1 (@%2)").arg(m_displayName, m_currentUsername));
    resize(980, 680);
    setMinimumSize(640, 480);

    // Модели данных
    m_chatModel = new ChatModel(this);
    m_chatModel->loadFromDB();

    m_chatProxyModel = new QSortFilterProxyModel(this);
    m_chatProxyModel->setSourceModel(m_chatModel);
    m_chatProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    m_messageModel = new MessageModel(this);

    // Бот-симулятор для локальных ботов
    m_botSimulator = new BotSimulator(this);
    connect(m_botSimulator, &BotSimulator::typingStatusChanged, this, &MainWindow::onBotTypingChanged);
    connect(m_botSimulator, &BotSimulator::replyReady, this, &MainWindow::onBotReplyReady);

    // Всплывающее окно эмодзи
    m_emojiPicker = new EmojiPickerPopup(this);
    connect(m_emojiPicker, &EmojiPickerPopup::emojiSelected, this, &MainWindow::onEmojiSelected);

    // Таймер сброса статуса печати
    m_typingTimer = new QTimer(this);
    m_typingTimer->setSingleShot(true);
    m_typingTimer->setInterval(2000);
    connect(m_typingTimer, &QTimer::timeout, this, [this]() {
        if (m_isTypingSent && m_currentChatId != -1) {
            m_isTypingSent = false;
            NetworkClient::instance().sendTyping(m_currentChatName, false);
        }
    });

    // Подключение сигналов сети
    auto &client = NetworkClient::instance();
    connect(&client, &NetworkClient::messageReceived, this, &MainWindow::onNetworkMessageReceived);
    connect(&client, &NetworkClient::typingReceived, this, &MainWindow::onNetworkTypingReceived);
    connect(&client, &NetworkClient::userListReceived, this, &MainWindow::onNetworkUserListReceived);

    setupUI();

    // Запрашиваем актуальный список пользователей
    client.requestUserList();

    // Открываем первый чат по умолчанию, если есть
    if (m_chatProxyModel->rowCount() > 0) {
        QModelIndex firstIdx = m_chatProxyModel->index(0, 0);
        m_chatList->setCurrentIndex(firstIdx);
        onChatClicked(firstIdx);
    }
}

void Messenger::MainWindow::setupStyles() {
    QFile styleFile(":/style.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        setStyleSheet(QString::fromUtf8(styleFile.readAll()));
    }
}

void Messenger::MainWindow::setupUI() {
    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setHandleWidth(1);

    // ==========================================
    // ЛЕВАЯ ПАНЕЛЬ: список чатов и поиск
    // ==========================================
    auto *leftWidget = new QWidget(splitter);
    leftWidget->setObjectName("leftPanel");
    auto *leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(0);

    // Шапка левой панели
    auto *leftHeader = new QWidget(leftWidget);
    leftHeader->setObjectName("leftHeader");
    leftHeader->setFixedHeight(56);
    auto *leftHeaderLayout = new QHBoxLayout(leftHeader);
    leftHeaderLayout->setContentsMargins(10, 8, 10, 8);
    leftHeaderLayout->setSpacing(8);

    m_menuBtn = new QPushButton("☰", leftHeader);
    m_menuBtn->setObjectName("menuBtn");
    m_menuBtn->setFixedSize(36, 36);
    m_menuBtn->setCursor(Qt::PointingHandCursor);

    auto *hamburgerMenu = new QMenu(this);
    auto *newChatAction = hamburgerMenu->addAction("➕ Новый чат...");
    auto *savedAction = hamburgerMenu->addAction("🔖 Избранное");
    hamburgerMenu->addSeparator();
    auto *aboutAction = hamburgerMenu->addAction("ℹ️ О программе Telegram Clone");

    connect(m_menuBtn, &QPushButton::clicked, this, [this, hamburgerMenu]() {
        hamburgerMenu->exec(m_menuBtn->mapToGlobal(QPoint(0, m_menuBtn->height())));
    });
    connect(newChatAction, &QAction::triggered, this, &MainWindow::onNewChatClicked);
    connect(savedAction, &QAction::triggered, this, [this]() {
        for (int r = 0; r < m_chatProxyModel->rowCount(); ++r) {
            QModelIndex idx = m_chatProxyModel->index(r, 0);
            if (idx.data(Qt::DisplayRole).toString() == "Избранное") {
                m_chatList->setCurrentIndex(idx);
                onChatClicked(idx);
                break;
            }
        }
    });
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAboutDialog);

    m_searchEdit = new QLineEdit(leftHeader);
    m_searchEdit->setObjectName("searchEdit");
    m_searchEdit->setPlaceholderText("🔍 Поиск");
    m_searchEdit->setClearButtonEnabled(true);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);

    m_newChatBtn = new QPushButton("➕", leftHeader);
    m_newChatBtn->setObjectName("newChatBtn");
    m_newChatBtn->setToolTip("Создать новый чат");
    m_newChatBtn->setFixedSize(36, 36);
    m_newChatBtn->setCursor(Qt::PointingHandCursor);
    connect(m_newChatBtn, &QPushButton::clicked, this, &MainWindow::onNewChatClicked);

    leftHeaderLayout->addWidget(m_menuBtn, 0, Qt::AlignCenter);
    leftHeaderLayout->addWidget(m_searchEdit, 1);
    leftHeaderLayout->addWidget(m_newChatBtn, 0, Qt::AlignCenter);

    // Список чатов
    m_chatList = new QListView(leftWidget);
    m_chatList->setObjectName("chatListView");
    m_chatList->setModel(m_chatProxyModel);
    m_chatList->setItemDelegate(new ChatDelegate(this));
    m_chatList->setFrameShape(QFrame::NoFrame);
    m_chatList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_chatList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_chatList, &QListView::clicked, this, &MainWindow::onChatClicked);
    connect(m_chatList, &QListView::customContextMenuRequested, this, &MainWindow::onChatContextMenu);

    leftLayout->addWidget(leftHeader);
    leftLayout->addWidget(m_chatList);

    // ==========================================
    // ПРАВАЯ ПАНЕЛЬ: активный чат
    // ==========================================
    auto *rightWidget = new QWidget(splitter);
    rightWidget->setObjectName("rightPanel");
    auto *rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    // Шапка активного чата
    m_headerWidget = new QWidget(rightWidget);
    m_headerWidget->setObjectName("chatHeader");
    m_headerWidget->setFixedHeight(56);
    auto *headerLayout = new QHBoxLayout(m_headerWidget);
    headerLayout->setContentsMargins(16, 8, 16, 8);
    headerLayout->setSpacing(12);

    m_chatAvatarLabel = new QLabel(m_headerWidget);
    m_chatAvatarLabel->setFixedSize(38, 38);
    m_chatAvatarLabel->setStyleSheet("background-color: transparent;");

    auto *titlesWidget = new QWidget(m_headerWidget);
    titlesWidget->setObjectName("titlesWidget");
    titlesWidget->setStyleSheet("background-color: transparent;");
    auto *titlesLayout = new QVBoxLayout(titlesWidget);
    titlesLayout->setContentsMargins(0, 0, 0, 0);
    titlesLayout->setSpacing(2);

    m_chatTitle = new QLabel("Выберите чат", titlesWidget);
    m_chatTitle->setObjectName("chatTitleLabel");
    m_chatTitle->setStyleSheet("background-color: transparent; color: #ffffff;");
    QFont titleFont = m_chatTitle->font();
    titleFont.setBold(true);
    titleFont.setPointSize(11);
    m_chatTitle->setFont(titleFont);

    m_chatSubtitle = new QLabel(titlesWidget);
    m_chatSubtitle->setObjectName("chatSubtitleLabel");
    m_chatSubtitle->setStyleSheet("background-color: transparent; color: #7f91a4;");
    QFont subFont = m_chatSubtitle->font();
    subFont.setPointSize(9);
    m_chatSubtitle->setFont(subFont);

    titlesLayout->addWidget(m_chatTitle);
    titlesLayout->addWidget(m_chatSubtitle);

    m_searchInChatBtn = new QPushButton("🔍", m_headerWidget);
    m_searchInChatBtn->setObjectName("iconButton");
    m_searchInChatBtn->setToolTip("Поиск в этом чате");
    m_searchInChatBtn->setFixedSize(36, 36);
    m_searchInChatBtn->setCursor(Qt::PointingHandCursor);
    connect(m_searchInChatBtn, &QPushButton::clicked, this, &MainWindow::onToggleInChatSearch);

    m_chatOptionsBtn = new QPushButton("⋮", m_headerWidget);
    m_chatOptionsBtn->setObjectName("iconButton");
    m_chatOptionsBtn->setToolTip("Действия с чатом");
    m_chatOptionsBtn->setFixedSize(36, 36);
    m_chatOptionsBtn->setCursor(Qt::PointingHandCursor);

    auto *chatOptionsMenu = new QMenu(this);
    auto *pinChatAction = chatOptionsMenu->addAction("📌 Закрепить / Открепить");
    auto *clearHistoryAction = chatOptionsMenu->addAction("🗑️ Очистить историю");
    auto *deleteChatAction = chatOptionsMenu->addAction("❌ Удалить чат");

    connect(m_chatOptionsBtn, &QPushButton::clicked, this, [this, chatOptionsMenu]() {
        if (m_currentChatId != -1) {
            chatOptionsMenu->exec(m_chatOptionsBtn->mapToGlobal(QPoint(0, m_chatOptionsBtn->height())));
        }
    });

    connect(pinChatAction, &QAction::triggered, this, [this]() {
        if (m_currentChatId != -1) {
            m_chatModel->togglePinChat(m_currentChatId);
        }
    });

    connect(clearHistoryAction, &QAction::triggered, this, [this]() {
        if (m_currentChatId != -1) {
            if (QMessageBox::question(this, "Очистить историю", "Вы действительно хотите удалить все сообщения в этом чате?") == QMessageBox::Yes) {
                m_chatModel->clearChatHistory(m_currentChatId);
                m_messageModel->clear();
            }
        }
    });

    connect(deleteChatAction, &QAction::triggered, this, [this]() {
        if (m_currentChatId != -1) {
            if (QMessageBox::question(this, "Удалить чат", "Удалить этот чат и всю переписку?") == QMessageBox::Yes) {
                int chatId = m_currentChatId;
                m_chatModel->deleteChatById(chatId);
                m_currentChatId = -1;
                m_chatTitle->setText("Выберите чат");
                m_chatSubtitle->clear();
                m_messageModel->clear();
            }
        }
    });

    headerLayout->addWidget(m_chatAvatarLabel, 0, Qt::AlignCenter);
    headerLayout->addWidget(titlesWidget, 1);
    headerLayout->addWidget(m_searchInChatBtn, 0, Qt::AlignCenter);
    headerLayout->addWidget(m_chatOptionsBtn, 0, Qt::AlignCenter);

    // Панель поиска сообщений внутри чата
    m_inChatSearchWidget = new QWidget(rightWidget);
    m_inChatSearchWidget->setObjectName("inChatSearchWidget");
    m_inChatSearchWidget->setFixedHeight(44);
    m_inChatSearchWidget->setVisible(false);

    auto *searchLayout = new QHBoxLayout(m_inChatSearchWidget);
    searchLayout->setContentsMargins(16, 4, 16, 4);
    searchLayout->setSpacing(8);

    m_inChatSearchEdit = new QLineEdit(m_inChatSearchWidget);
    m_inChatSearchEdit->setPlaceholderText("Поиск по сообщениям...");
    m_inChatSearchEdit->setClearButtonEnabled(true);
    connect(m_inChatSearchEdit, &QLineEdit::textChanged, this, &MainWindow::onInChatSearchChanged);

    m_inChatSearchCountLabel = new QLabel(m_inChatSearchWidget);
    m_inChatSearchCountLabel->setStyleSheet("color: #7f91a4; font-size: 11px; background: transparent;");

    auto *closeSearchBtn = new QPushButton("✕", m_inChatSearchWidget);
    closeSearchBtn->setObjectName("iconButton");
    closeSearchBtn->setFixedSize(28, 28);
    closeSearchBtn->setCursor(Qt::PointingHandCursor);
    connect(closeSearchBtn, &QPushButton::clicked, this, &MainWindow::onToggleInChatSearch);

    searchLayout->addWidget(m_inChatSearchEdit);
    searchLayout->addWidget(m_inChatSearchCountLabel);
    searchLayout->addWidget(closeSearchBtn);

    // Список сообщений
    m_messagesView = new QListView(rightWidget);
    m_messagesView->setObjectName("messagesView");
    m_messagesView->setModel(m_messageModel);
    m_messagesView->setItemDelegate(new MessageDelegate(this));
    m_messagesView->setSelectionMode(QAbstractItemView::NoSelection);
    m_messagesView->setFocusPolicy(Qt::NoFocus);
    m_messagesView->setFrameShape(QFrame::NoFrame);
    m_messagesView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_messagesView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_messagesView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_messagesView, &QListView::customContextMenuRequested, this, &MainWindow::onMessageContextMenu);

    // Баннер ответа / редактирования
    m_actionBanner = new QWidget(rightWidget);
    m_actionBanner->setObjectName("actionBanner");
    m_actionBanner->setFixedHeight(42);
    m_actionBanner->setVisible(false);

    auto *bannerLayout = new QHBoxLayout(m_actionBanner);
    bannerLayout->setContentsMargins(16, 4, 16, 4);
    bannerLayout->setSpacing(10);

    auto *bannerBar = new QFrame(m_actionBanner);
    bannerBar->setFixedWidth(3);
    bannerBar->setStyleSheet("background-color: #5494d4; border-radius: 1px;");

    auto *bannerTextWidget = new QWidget(m_actionBanner);
    bannerTextWidget->setObjectName("bannerTextWidget");
    bannerTextWidget->setStyleSheet("background-color: transparent;");
    auto *bannerTextLayout = new QVBoxLayout(bannerTextWidget);
    bannerTextLayout->setContentsMargins(0, 0, 0, 0);
    bannerTextLayout->setSpacing(1);

    m_bannerTitleLabel = new QLabel("В ответ на:", bannerTextWidget);
    m_bannerTitleLabel->setStyleSheet("color: #5494d4; font-weight: bold; font-size: 11px; background: transparent;");
    m_bannerTextLabel = new QLabel(bannerTextWidget);
    m_bannerTextLabel->setStyleSheet("color: #cfdbe6; font-size: 11px; background: transparent;");

    bannerTextLayout->addWidget(m_bannerTitleLabel);
    bannerTextLayout->addWidget(m_bannerTextLabel);

    m_bannerCloseBtn = new QPushButton("✕", m_actionBanner);
    m_bannerCloseBtn->setObjectName("iconButton");
    m_bannerCloseBtn->setFixedSize(24, 24);
    m_bannerCloseBtn->setCursor(Qt::PointingHandCursor);
    connect(m_bannerCloseBtn, &QPushButton::clicked, this, &MainWindow::onCancelReplyOrEdit);

    bannerLayout->addWidget(bannerBar);
    bannerLayout->addWidget(bannerTextWidget);
    bannerLayout->addStretch();
    bannerLayout->addWidget(m_bannerCloseBtn);

    // Нижняя панель ввода сообщения
    m_inputBar = new QWidget(rightWidget);
    m_inputBar->setObjectName("inputBar");
    m_inputBar->setFixedHeight(58);

    auto *inputLayout = new QHBoxLayout(m_inputBar);
    inputLayout->setContentsMargins(12, 8, 12, 8);
    inputLayout->setSpacing(8);

    m_attachBtn = new QPushButton("📎", m_inputBar);
    m_attachBtn->setObjectName("iconButton");
    m_attachBtn->setToolTip("Прикрепить фото или файл");
    m_attachBtn->setFixedSize(36, 36);
    m_attachBtn->setCursor(Qt::PointingHandCursor);
    connect(m_attachBtn, &QPushButton::clicked, this, &MainWindow::onAttachFileClicked);

    m_messageInput = new QLineEdit(m_inputBar);
    m_messageInput->setObjectName("messageInput");
    m_messageInput->setPlaceholderText("Написать сообщение...");
    m_messageInput->installEventFilter(this);
    connect(m_messageInput, &QLineEdit::returnPressed, this, &MainWindow::onSendBtnClicked);
    connect(m_messageInput, &QLineEdit::textEdited, this, &MainWindow::onInputTextChanged);

    m_emojiBtn = new QPushButton("😊", m_inputBar);
    m_emojiBtn->setObjectName("iconButton");
    m_emojiBtn->setToolTip("Эмодзи");
    m_emojiBtn->setFixedSize(36, 36);
    m_emojiBtn->setCursor(Qt::PointingHandCursor);
    connect(m_emojiBtn, &QPushButton::clicked, this, &MainWindow::onEmojiButtonClicked);

    m_sendButton = new QPushButton("➤", m_inputBar);
    m_sendButton->setObjectName("sendButton");
    m_sendButton->setToolTip("Отправить");
    m_sendButton->setFixedSize(38, 38);
    m_sendButton->setCursor(Qt::PointingHandCursor);
    connect(m_sendButton, &QPushButton::clicked, this, &MainWindow::onSendBtnClicked);

    inputLayout->addWidget(m_attachBtn, 0, Qt::AlignCenter);
    inputLayout->addWidget(m_messageInput, 1);
    inputLayout->addWidget(m_emojiBtn, 0, Qt::AlignCenter);
    inputLayout->addWidget(m_sendButton, 0, Qt::AlignCenter);

    rightLayout->addWidget(m_headerWidget);
    rightLayout->addWidget(m_inChatSearchWidget);
    rightLayout->addWidget(m_messagesView);
    rightLayout->addWidget(m_actionBanner);
    rightLayout->addWidget(m_inputBar);

    splitter->addWidget(leftWidget);
    splitter->addWidget(rightWidget);
    splitter->setSizes({300, 680});

    mainLayout->addWidget(splitter);
}

bool Messenger::MainWindow::eventFilter(QObject *watched, QEvent *event) {
    if (watched == m_messageInput && event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            onCancelReplyOrEdit();
            return true;
        }
        if (keyEvent->key() == Qt::Key_Up && m_messageInput->text().isEmpty() && m_editingMessageId == 0) {
            Message lastOut = m_messageModel->lastOutgoingMessage();
            if (lastOut.id > 0) {
                startEditMessage(lastOut);
                return true;
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

void Messenger::MainWindow::onInputTextChanged(const QString &text) {
    Q_UNUSED(text)
    if (m_currentChatId != -1 && m_currentChatType == "user") {
        if (!m_isTypingSent) {
            m_isTypingSent = true;
            NetworkClient::instance().sendTyping(m_currentChatName, true);
        }
        m_typingTimer->start();
    }
}

void Messenger::MainWindow::onChatClicked(const QModelIndex &index) {
    if (!index.isValid())
        return;

    int chatId = index.data(Messenger::ChatRoles::idRole).toInt();
    QString chatName = index.data(Qt::DisplayRole).toString();
    QString chatType = index.data(Messenger::ChatRoles::ChatTypeRole).toString();

    openChat(chatId, chatName, chatType);
}

void Messenger::MainWindow::openChat(int chatId, const QString &chatName, const QString &chatType) {
    m_currentChatId = chatId;
    m_currentChatName = chatName;
    m_currentChatType = chatType;

    m_chatTitle->setText(chatName);

    if (chatType == "saved" || chatName == "Избранное") {
        m_chatSubtitle->setText("ваше личное облако");
    } else if (chatType == "bot") {
        m_chatSubtitle->setText("бот");
    } else if (chatType == "channel") {
        m_chatSubtitle->setText("канал");
    } else {
        m_chatSubtitle->setText("в сети");
    }

    // Круглая аватарка в шапке
    QPixmap avatarPix(38, 38);
    avatarPix.fill(Qt::transparent);
    QPainter p(&avatarPix);
    p.setRenderHint(QPainter::Antialiasing);

    QColor avColor = (chatType == "saved" || chatName == "Избранное") ? QColor("#2AABEE") : QColor("#5494d4");
    p.setPen(Qt::NoPen);
    p.setBrush(avColor);
    p.drawEllipse(0, 0, 38, 38);

    p.setPen(Qt::white);
    QFont f = p.font();
    f.setBold(true);
    f.setPointSize(chatType == "saved" ? 14 : 12);
    p.setFont(f);
    if (chatType == "saved" || chatName == "Избранное") {
        p.drawText(avatarPix.rect(), Qt::AlignCenter, "🔖");
    } else {
        p.drawText(avatarPix.rect(), Qt::AlignCenter, chatName.left(1).toUpper());
    }
    m_chatAvatarLabel->setPixmap(avatarPix);

    // Сбрасываем непрочитанные
    m_chatModel->markChatAsRead(chatId);

    // Загружаем историю сообщений
    m_messageModel->setMessages(m_chatModel->messages(chatId));

    // Сбрасываем режим ответа / редактирования
    onCancelReplyOrEdit();

    m_messagesView->scrollToBottom();
    m_messageInput->setFocus();
}

void Messenger::MainWindow::onSendBtnClicked() {
    if (m_currentChatId == -1)
        return;

    QString text = m_messageInput->text().trimmed();
    if (text.isEmpty() && m_pendingAttachmentPath.isEmpty())
        return;

    // Режим редактирования
    if (m_editingMessageId != 0) {
        m_messageModel->updateMessage(m_editingMessageId, text);
        m_chatModel->updateMessage(m_currentChatId, m_editingMessageId, text);
        onCancelReplyOrEdit();
        return;
    }

    // Режим отправки нового сообщения
    Message msg(text, true, QDateTime::currentDateTime().toString("HH:mm"),
                m_pendingAttachmentPath, m_replyToId, m_replyText);

    m_chatModel->addMessage(m_currentChatId, msg);
    m_messageModel->addMessage(msg);

    QString sentText = text;
    QString sentAttach = m_pendingAttachmentPath;
    QString sentReply = m_replyText;

    m_messageInput->clear();
    onCancelReplyOrEdit();
    m_messagesView->scrollToBottom();

    // Отправка по сети, если это реальный сетевой пользователь
    if (m_currentChatType == "user" && m_currentChatName != "Избранное" && m_currentChatName != "Павел Дуров") {
        NetworkClient::instance().sendMessage(m_currentChatName, sentText, sentAttach, 0, sentReply);
    } else {
        // Локальная симуляция для бота и демо-контактов
        m_botSimulator->processOutgoingMessage(m_currentChatId, m_currentChatName, m_currentChatType, sentText);
    }
}

int Messenger::MainWindow::getOrCreateChatForUser(const QString &username, const QString &displayName) {
    QString title = displayName.isEmpty() ? username : displayName;
    for (const auto &chat : m_chatModel->chats()) {
        if (chat.name.compare(title, Qt::CaseInsensitive) == 0 ||
            chat.name.compare(username, Qt::CaseInsensitive) == 0) {
            return chat.id;
        }
    }

    // Создаем новый чат для контакта
    int newId = m_chatModel->nextChatId();
    m_chatModel->addChat(newId, title, "user", false);
    return newId;
}

void Messenger::MainWindow::onNetworkMessageReceived(const QString &fromUser, const QString &toUser,
                                                     const QString &text, const QString &timestamp,
                                                     const QString &filePath, const QString &replyText) {
    Q_UNUSED(toUser)
    int chatId = getOrCreateChatForUser(fromUser);

    Message msg(text, false, timestamp, filePath, 0, replyText);
    m_chatModel->addMessage(chatId, msg);

    if (chatId == m_currentChatId) {
        m_messageModel->addMessage(msg);
        m_messagesView->scrollToBottom();
    } else {
        ChatInfo info = m_chatModel->chatInfo(chatId);
        m_chatModel->setUnreadCount(chatId, info.unreadCount + 1);
    }
}

void Messenger::MainWindow::onNetworkTypingReceived(const QString &fromUser, bool isTyping) {
    if (m_currentChatName.compare(fromUser, Qt::CaseInsensitive) == 0) {
        if (isTyping) {
            m_chatSubtitle->setText("печатает...");
            m_chatSubtitle->setStyleSheet("color: #4fae63; font-weight: bold; background: transparent;");
        } else {
            m_chatSubtitle->setStyleSheet("color: #7f91a4; background: transparent;");
            m_chatSubtitle->setText("в сети");
        }
    }
}

void Messenger::MainWindow::onNetworkUserListReceived(const QList<RemoteUser> &users) {
    // При получении списка пользователей обновляем статус контакта, если чат открыт
    for (const auto &u : users) {
        if (m_currentChatName.compare(u.username, Qt::CaseInsensitive) == 0 ||
            m_currentChatName.compare(u.displayName, Qt::CaseInsensitive) == 0) {
            if (m_currentChatType == "user") {
                m_chatSubtitle->setText(u.isOnline ? "в сети" : "был(а) недавно");
            }
        }
    }
}

void Messenger::MainWindow::startReplyMessage(const Message &msg) {
    m_editingMessageId = 0;
    m_replyToId = msg.id;
    m_replyText = msg.text.isEmpty() ? "📷 Фото" : msg.text;

    m_bannerTitleLabel->setText("В ответ на:");
    m_bannerTextLabel->setText(m_replyText);
    m_actionBanner->setVisible(true);

    m_sendButton->setText("➤");
    m_messageInput->setFocus();
}

void Messenger::MainWindow::startEditMessage(const Message &msg) {
    m_replyToId = 0;
    m_replyText.clear();
    m_editingMessageId = msg.id;

    m_bannerTitleLabel->setText("Редактирование");
    m_bannerTextLabel->setText(msg.text);
    m_actionBanner->setVisible(true);

    m_messageInput->setText(msg.text);
    m_messageInput->selectAll();
    m_messageInput->setFocus();
    m_sendButton->setText("✓");
}

void Messenger::MainWindow::onCancelReplyOrEdit() {
    m_replyToId = 0;
    m_replyText.clear();
    m_editingMessageId = 0;
    m_pendingAttachmentPath.clear();
    m_actionBanner->setVisible(false);
    m_sendButton->setText("➤");
    m_attachBtn->setStyleSheet("");
}

void Messenger::MainWindow::onAttachFileClicked() {
    QString filePath = QFileDialog::getOpenFileName(
        this, "Выберите изображение для отправки", QString(),
        "Изображения (*.png *.jpg *.jpeg *.bmp *.gif *.webp);;Все файлы (*.*)");

    if (!filePath.isEmpty()) {
        m_pendingAttachmentPath = filePath;
        m_attachBtn->setStyleSheet("background-color: #2b5278;");

        m_bannerTitleLabel->setText("Прикреплен файл:");
        m_bannerTextLabel->setText(QFileInfo(filePath).fileName());
        m_actionBanner->setVisible(true);

        m_messageInput->setFocus();
    }
}

void Messenger::MainWindow::onEmojiButtonClicked() {
    QPoint btnPos = m_emojiBtn->mapToGlobal(QPoint(0, 0));
    m_emojiPicker->move(btnPos.x() - 120, btnPos.y() - m_emojiPicker->height() - 8);
    m_emojiPicker->show();
}

void Messenger::MainWindow::onEmojiSelected(const QString &emoji) {
    m_messageInput->insert(emoji);
    m_messageInput->setFocus();
}

void Messenger::MainWindow::onSearchTextChanged(const QString &text) {
    m_chatProxyModel->setFilterRegularExpression(QRegularExpression::escape(text));
}

void Messenger::MainWindow::onToggleInChatSearch() {
    bool visible = !m_inChatSearchWidget->isVisible();
    m_inChatSearchWidget->setVisible(visible);
    if (visible) {
        m_inChatSearchEdit->clear();
        m_inChatSearchEdit->setFocus();
    }
}

void Messenger::MainWindow::onInChatSearchChanged(const QString &text) {
    if (text.trimmed().isEmpty()) {
        m_inChatSearchCountLabel->clear();
        return;
    }

    int matchCount = 0;
    int firstMatchRow = -1;
    for (int i = 0; i < m_messageModel->rowCount(); ++i) {
        Message msg = m_messageModel->messageAt(i);
        if (msg.text.contains(text, Qt::CaseInsensitive)) {
            matchCount++;
            if (firstMatchRow == -1) {
                firstMatchRow = i;
            }
        }
    }

    m_inChatSearchCountLabel->setText(QString("Найдено: %1").arg(matchCount));
    if (firstMatchRow != -1) {
        m_messagesView->scrollTo(m_messageModel->index(firstMatchRow, 0), QAbstractItemView::PositionAtCenter);
    }
}

void Messenger::MainWindow::onBotTypingChanged(int chatId, bool isTyping) {
    if (chatId == m_currentChatId) {
        if (isTyping) {
            m_chatSubtitle->setText("печатает...");
            m_chatSubtitle->setStyleSheet("color: #4fae63; font-weight: bold; background: transparent;");
        } else {
            m_chatSubtitle->setStyleSheet("color: #7f91a4; background: transparent;");
            if (m_currentChatType == "saved" || m_currentChatName == "Избранное") {
                m_chatSubtitle->setText("ваше личное облако");
            } else if (m_currentChatType == "bot") {
                m_chatSubtitle->setText("бот");
            } else {
                m_chatSubtitle->setText("в сети");
            }
        }
    }
}

void Messenger::MainWindow::onBotReplyReady(int chatId, const Message &reply) {
    Message copy = reply;
    m_chatModel->addMessage(chatId, copy);

    if (chatId == m_currentChatId) {
        m_messageModel->addMessage(copy);
        m_messagesView->scrollToBottom();
    } else {
        ChatInfo info = m_chatModel->chatInfo(chatId);
        m_chatModel->setUnreadCount(chatId, info.unreadCount + 1);
    }
}

void Messenger::MainWindow::onNewChatClicked() {
    NewChatDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        QString name = dlg.chatName();
        QString type = dlg.chatType();
        QString initMsg = dlg.initialMessage();

        int newId = getOrCreateChatForUser(name, name);

        if (!initMsg.isEmpty()) {
            Message msg(initMsg, true, QDateTime::currentDateTime().toString("HH:mm"));
            m_chatModel->addMessage(newId, msg);
            if (type == "user" && name != "Избранное") {
                NetworkClient::instance().sendMessage(name, initMsg);
            }
        }

        // Находим и открываем созданный чат
        for (int r = 0; r < m_chatProxyModel->rowCount(); ++r) {
            QModelIndex idx = m_chatProxyModel->index(r, 0);
            if (idx.data(Messenger::ChatRoles::idRole).toInt() == newId) {
                m_chatList->setCurrentIndex(idx);
                onChatClicked(idx);
                break;
            }
        }
    }
}

void Messenger::MainWindow::onChatContextMenu(const QPoint &pos) {
    QModelIndex proxyIdx = m_chatList->indexAt(pos);
    if (!proxyIdx.isValid())
        return;

    int chatId = proxyIdx.data(Messenger::ChatRoles::idRole).toInt();
    bool isPinned = proxyIdx.data(Messenger::ChatRoles::IsPinnedRole).toBool();

    QMenu menu(this);
    QAction *pinAction = menu.addAction(isPinned ? "📌 Открепить чат" : "📌 Закрепить чат");
    QAction *clearAction = menu.addAction("🗑️ Очистить историю");
    QAction *deleteAction = menu.addAction("❌ Удалить чат");

    QAction *selected = menu.exec(m_chatList->mapToGlobal(pos));
    if (selected == pinAction) {
        m_chatModel->togglePinChat(chatId);
    } else if (selected == clearAction) {
        if (QMessageBox::question(this, "Очистить историю", "Удалить все сообщения?") == QMessageBox::Yes) {
            m_chatModel->clearChatHistory(chatId);
            if (chatId == m_currentChatId) {
                m_messageModel->clear();
            }
        }
    } else if (selected == deleteAction) {
        if (QMessageBox::question(this, "Удалить чат", "Удалить этот чат?") == QMessageBox::Yes) {
            m_chatModel->deleteChatById(chatId);
            if (chatId == m_currentChatId) {
                m_currentChatId = -1;
                m_chatTitle->setText("Выберите чат");
                m_chatSubtitle->clear();
                m_messageModel->clear();
            }
        }
    }
}

void Messenger::MainWindow::onMessageContextMenu(const QPoint &pos) {
    QModelIndex idx = m_messagesView->indexAt(pos);
    if (!idx.isValid())
        return;

    Message msg = m_messageModel->messageAt(idx.row());

    QMenu menu(this);
    QAction *replyAction = menu.addAction("↩️ Ответить");
    QAction *copyAction = menu.addAction("📋 Копировать текст");
    QAction *editAction = nullptr;
    if (msg.isOutgoing && !msg.text.isEmpty()) {
        editAction = menu.addAction("✏️ Редактировать");
    }
    QAction *deleteAction = menu.addAction("🗑️ Удалить сообщение");

    QAction *selected = menu.exec(m_messagesView->mapToGlobal(pos));
    if (selected == replyAction) {
        startReplyMessage(msg);
    } else if (selected == copyAction) {
        QApplication::clipboard()->setText(msg.text);
    } else if (editAction && selected == editAction) {
        startEditMessage(msg);
    } else if (selected == deleteAction) {
        m_chatModel->deleteMessage(m_currentChatId, msg.id);
        m_messageModel->removeMessage(msg.id);
    }
}

void Messenger::MainWindow::showAboutDialog() {
    QMessageBox::about(this, "Telegram Clone",
                       "<h3>Telegram Clone Desktop</h3>"
                       "<p>Полнофункциональный сетевой мессенджер в стиле Telegram на базе C++ и Qt 6.</p>"
                       "<p><b>Возможности:</b></p>"
                       "<ul>"
                       "<li>Работа по сети между экземплярами приложения</li>"
                       "<li>Регистрация и вход под разными профилями</li>"
                       "<li>Личные диалоги, группы, каналы и боты</li>"
                       "<li>Мгновенный статус «печатает...» и уведомления</li>"
                       "<li>Закрепление чатов и фильтрация поиска</li>"
                       "<li>Цитирование (ответы) и редактирование сообщений</li>"
                       "<li>Отправка изображений и всплывающая панель эмодзи</li>"
                       "<li>Сохранение всей переписки в базе SQLite</li>"
                       "</ul>");
}
