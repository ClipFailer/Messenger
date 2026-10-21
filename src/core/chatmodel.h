#ifndef CHATMODEL_H
#define CHATMODEL_H

#include <QAbstractListModel>
#include <QMap>
#include <QString>
#include <QStringList>

namespace Messenger {

struct Message;

/// Структура, описывающая базовые данные чата для левой панели.
struct ChatInfo {
    int     id;   //< Уникальный идентификатор чата.
    QString name; //< название чата.
};

/// Перечисление для получения данных от модели.
enum ChatRoles {
    idRole          = Qt::UserRole + 1, //< роль для получения айди чата.
    LastMessageRole = Qt::UserRole + 2  //< роль для получения последнего собщения чата.
};

/**
 * @brief Модель данных списка чатов.
 *
 * Хранит историю сообщений и порядок чатов.
 */
class ChatModel : public QAbstractListModel {
    Q_OBJECT

  public:
    explicit ChatModel(QObject *parent = nullptr);

    //====== Перегрузки ======
    int      rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    //======= Геттеры =======
    /// Возвращает список чатов.
    QList<ChatInfo> chats() const;

    /// Возвращает все сообщения чата по идентификатору.
    QList<Message> messages(const int chatId) const;

    /// Проверяет, существует ли чат по идентификатору.
    bool hasChat(const int chatId) const;

    /**
     * @brief Создает новый чат.
     *
     * @param id Идентификатор чата
     * @param chatName Название чата
     */
    void addChat(const int chatId, const QString &chatName);

    /**
     * @brief Удаляет чат.
     *
     * @param row Номер строки в модели
     */
    void deleteChat(const int row);

    /**
     * @brief Добавляет сообщение в чат.
     *
     * @param id Идентификатор чата
     * @param Сообщение для отправки
     */
    void addMessage(const int chatId, const Message &message);

  private:
    QList<ChatInfo>           m_chats;    //< список чатов.
    QMap<int, QList<Message>> m_messages; //< мэп всех сообщений чата по айди.
};
} // namespace Messenger

#endif // CHATMODEL_H
