#ifndef MESSAGEMODEL_H
#define MESSAGEMODEL_H

#include <QAbstractListModel> // ← ListModel, не ItemModel
#include <QStringList>

namespace Messenger {
enum Roles { IsOutgoingRole = Qt::UserRole + 1 };

/**
 * @brief Структура, описывающая объект сообщения в чате.
 */
struct Message {
    Message(const QString &text, bool isOutgoing = true) : text(text), isOutgoing(isOutgoing) {}

    QString text;       //< Текст сообщения.
    bool    isOutgoing; //< Для проверки, отправителя сообщения.
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
     *
     * @param messages список сообщений чата.
     */
    void setMessages(const QList<Message> &messages);

    /**
     * @brief Добавляет сообщение в чат.
     *
     * @param message новое сообщение чата.
     */
    void addMessage(const Message &message);

    /// Очищает весь чат.
    void clear();

  private:
    QList<Message> m_messages; //< список сообщений чата.
};
} // namespace Messenger

#endif // MESSAGEMODEL_H
