#ifndef BOTSIMULATOR_H
#define BOTSIMULATOR_H

#include "MessageModel.h"
#include <QObject>

namespace Messenger {

class BotSimulator : public QObject {
    Q_OBJECT
  public:
    explicit BotSimulator(QObject *parent = nullptr);

    /// Обрабатывает отправленное пользователем сообщение и запускает ответ, если применимо.
    void processOutgoingMessage(int chatId, const QString &chatName, const QString &chatType, const QString &messageText);

  signals:
    /// Сигнал изменения статуса "печатает...".
    void typingStatusChanged(int chatId, bool isTyping);

    /// Сигнал готовности ответного сообщения.
    void replyReady(int chatId, const Message &reply);

  private:
    QString generateBotResponse(const QString &input);
    QString generateDurovResponse(const QString &input);
    QString generateUserResponse(const QString &input);
    double evaluateSimpleExpr(const QString &expr, bool &ok);
};

} // namespace Messenger

#endif // BOTSIMULATOR_H
