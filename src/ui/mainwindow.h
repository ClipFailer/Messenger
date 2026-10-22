#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "core/ChatModel.h"
#include "core/MessageModel.h"
#include "network/NetworkProtocol.h"
#include <QSortFilterProxyModel>
#include <QTimer>
#include <QWidget>

class QLabel;
class QLineEdit;
class QListView;
class QPushButton;
class QSplitter;
class QFrame;
class QHBoxLayout;
class QVBoxLayout;

namespace Messenger {

class BotSimulator;
class EmojiPickerPopup;

/**
 * @brief Главное окно мессенджера Telegram Clone с поддержкой сети.
 */
class MainWindow : public QWidget {
    Q_OBJECT

  public:
    explicit MainWindow(const QString &username = "user", const QString &displayName = "Пользователь",
                        QWidget *parent = nullptr);

  protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

  private slots:
    void onSendBtnClicked();
    void onChatClicked(const QModelIndex &index);
    void onChatContextMenu(const QPoint &pos);
    void onMessageContextMenu(const QPoint &pos);
    void onSearchTextChanged(const QString &text);
    void onNewChatClicked();
    void onAttachFileClicked();
    void onEmojiButtonClicked();
    void onEmojiSelected(const QString &emoji);
    void onCancelReplyOrEdit();
    void onToggleInChatSearch();
    void onInChatSearchChanged(const QString &text);

    // Бот
    void onBotTypingChanged(int chatId, bool isTyping);
    void onBotReplyReady(int chatId, const Message &reply);

    // Сетевые слоты
    void onNetworkMessageReceived(const QString &fromUser, const QString &toUser, const QString &text,
                                  const QString &timestamp, const QString &filePath, const QString &replyText);
    void onNetworkTypingReceived(const QString &fromUser, bool isTyping);
    void onNetworkUserListReceived(const QList<RemoteUser> &users);
    void onInputTextChanged(const QString &text);

  private:
    void setupUI();
    void setupStyles();
    void openChat(int chatId, const QString &chatName, const QString &chatType);
    void startEditMessage(const Message &msg);
    void startReplyMessage(const Message &msg);
    void showAboutDialog();
    int  getOrCreateChatForUser(const QString &username, const QString &displayName = QString());

    // Пользователь
    QString m_currentUsername;
    QString m_displayName;

    // Модели
    ChatModel              *m_chatModel = nullptr;
    QSortFilterProxyModel  *m_chatProxyModel = nullptr;
    MessageModel           *m_messageModel = nullptr;
    BotSimulator           *m_botSimulator = nullptr;

    // Состояние текущего чата
    int     m_currentChatId = -1;
    QString m_currentChatName;
    QString m_currentChatType;

    // Таймер печати
    QTimer *m_typingTimer = nullptr;
    bool    m_isTypingSent = false;

    // Состояние ответа / редактирования
    int     m_replyToId = 0;
    QString m_replyText;
    int     m_editingMessageId = 0;
    QString m_pendingAttachmentPath;

    // Виджеты левой панели
    QLineEdit   *m_searchEdit = nullptr;
    QPushButton *m_menuBtn = nullptr;
    QPushButton *m_newChatBtn = nullptr;
    QListView   *m_chatList = nullptr;

    // Виджеты правой панели (шапка)
    QWidget     *m_headerWidget = nullptr;
    QLabel      *m_chatAvatarLabel = nullptr;
    QLabel      *m_chatTitle = nullptr;
    QLabel      *m_chatSubtitle = nullptr;
    QPushButton *m_searchInChatBtn = nullptr;
    QPushButton *m_chatOptionsBtn = nullptr;

    // Панель поиска внутри чата
    QWidget     *m_inChatSearchWidget = nullptr;
    QLineEdit   *m_inChatSearchEdit = nullptr;
    QLabel      *m_inChatSearchCountLabel = nullptr;

    // Список сообщений
    QListView   *m_messagesView = nullptr;

    // Баннер ответа / редактирования
    QWidget     *m_actionBanner = nullptr;
    QLabel      *m_bannerTitleLabel = nullptr;
    QLabel      *m_bannerTextLabel = nullptr;
    QPushButton *m_bannerCloseBtn = nullptr;

    // Поле ввода сообщения и кнопки
    QWidget     *m_inputBar = nullptr;
    QPushButton *m_attachBtn = nullptr;
    QLineEdit   *m_messageInput = nullptr;
    QPushButton *m_emojiBtn = nullptr;
    QPushButton *m_sendButton = nullptr;

    EmojiPickerPopup *m_emojiPicker = nullptr;
};

} // namespace Messenger

#endif // MAINWINDOW_H
