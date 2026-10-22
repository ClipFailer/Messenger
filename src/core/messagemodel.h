#ifndef MESSAGEMODEL_H
#define MESSAGEMODEL_H

#include <QAbstractListModel>
#include <QDateTime>
#include <QString>

namespace Messenger {

enum Roles {
    IdRole = Qt::UserRole + 1,
    IsOutgoingRole = Qt::UserRole + 2,
    TimestampRole = Qt::UserRole + 3,
    FilePathRole = Qt::UserRole + 4,
    ReplyToIdRole = Qt::UserRole + 5,
    ReplyTextRole = Qt::UserRole + 6,
    IsEditedRole = Qt::UserRole + 7,
    IsReadRole = Qt::UserRole + 8
};

/**
 * @brief Структура, описывающая объект сообщения в чате в стиле Telegram.
 */
struct Message {
    Message() = default;
    Message(const QString &text, bool isOutgoing = true,
            const QString &timestamp = QString(),
            const QString &filePath = QString(),
            int replyToId = 0,
            const QString &replyText = QString())
        : text(text)
        , isOutgoing(isOutgoing)
        , timestamp(timestamp.isEmpty() ? QDateTime::currentDateTime().toString("HH:mm") : timestamp)
        , filePath(filePath)
        , replyToId(replyToId)
        , replyText(replyText)
    {}

    int     id = 0;             //< Идентификатор сообщения в БД.
    int     chatId = 0;         //< Идентификатор чата.
    QString text;               //< Текст сообщения.
    bool    isOutgoing = true;  //< Флаг: отправлено нами (true) или входящее (false).
    QString timestamp;          //< Время отправки (например, "14:32").
    QString filePath;           //< Путь к прикрепленному файлу или изображению.
    int     replyToId = 0;      //< ID сообщения, на которое отвечаем.
    QString replyText;          //< Текст сообщения, на которое отвечаем.
    bool    isEdited = false;   //< Флаг: сообщение отредактировано.
    bool    isRead = true;      //< Статус прочтения.
};

/**
 * @brief Модель данных чата.
 */
class MessageModel : public QAbstractListModel {
    Q_OBJECT
  public:
    explicit MessageModel(QObject *parent = nullptr);

    //===== Перегрузки =====
    int      rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Задает сообщения чата.
     * @param messages список сообщений чата.
     */
    void setMessages(const QList<Message> &messages);

    /**
     * @brief Добавляет сообщение в чат.
     * @param message новое сообщение чата.
     */
    void addMessage(const Message &message);

    /**
     * @brief Обновляет текст сообщения.
     * @param id ID сообщения
     * @param newText Новый текст
     */
    bool updateMessage(int id, const QString &newText);

    /**
     * @brief Удаляет сообщение по ID.
     * @param id ID сообщения
     */
    bool removeMessage(int id);

    /**
     * @brief Возвращает сообщение по ID.
     */
    Message messageById(int id) const;

    /**
     * @brief Возвращает сообщение по индексу строки.
     */
    Message messageAt(int row) const;

    /**
     * @brief Возвращает последнее исходящее сообщение (для быстрого редактирования стрелкой вверх).
     */
    Message lastOutgoingMessage() const;

    /// Возвращает все сообщения.
    QList<Message> allMessages() const { return m_messages; }

    /// Очищает весь чат.
    void clear();

  private:
    QList<Message> m_messages; //< список сообщений чата.
};

} // namespace Messenger

#endif // MESSAGEMODEL_H
