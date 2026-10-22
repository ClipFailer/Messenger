#include "BotSimulator.h"
#include <QDateTime>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QTimer>

Messenger::BotSimulator::BotSimulator(QObject *parent)
    : QObject(parent)
{}

void Messenger::BotSimulator::processOutgoingMessage(int chatId, const QString &chatName,
                                                     const QString &chatType,
                                                     const QString &messageText) {
    // В "Избранном" автоответ не нужен
    if (chatType == "saved" || chatName == "Избранное") {
        return;
    }

    emit typingStatusChanged(chatId, true);

    int delayMs = 900 + QRandomGenerator::global()->bounded(600);

    QTimer::singleShot(delayMs, this, [this, chatId, chatName, chatType, messageText]() {
        emit typingStatusChanged(chatId, false);

        QString replyText;
        if (chatType == "bot" || chatName.contains("бот", Qt::CaseInsensitive)) {
            replyText = generateBotResponse(messageText);
        } else if (chatName.contains("Павел Дуров", Qt::CaseInsensitive)) {
            replyText = generateDurovResponse(messageText);
        } else {
            replyText = generateUserResponse(messageText);
        }

        Message reply(replyText, false, QDateTime::currentDateTime().toString("HH:mm"));
        emit replyReady(chatId, reply);
    });
}

double Messenger::BotSimulator::evaluateSimpleExpr(const QString &expr, bool &ok) {
    // Простой калькулятор базовых бинарных выражений: a + b, a - b, a * b, a / b
    static const QRegularExpression re(R"(^\s*([\d\.]+)\s*([\+\-\*\/])\s*([\d\.]+)\s*$)");
    auto match = re.match(expr);
    if (match.hasMatch()) {
        double a = match.captured(1).toDouble(&ok);
        if (!ok) return 0;
        QString op = match.captured(2);
        double b = match.captured(3).toDouble(&ok);
        if (!ok) return 0;

        if (op == "+") return a + b;
        if (op == "-") return a - b;
        if (op == "*") return a * b;
        if (op == "/") {
            if (qFuzzyIsNull(b)) {
                ok = false;
                return 0;
            }
            return a / b;
        }
    }
    ok = false;
    return 0;
}

QString Messenger::BotSimulator::generateBotResponse(const QString &input) {
    const QString trimmed = input.trimmed();

    if (trimmed.startsWith("/help", Qt::CaseInsensitive)) {
        return "🤖 Доступные команды бота:\n"
               "/help — список команд\n"
               "/time — текущее время и дата\n"
               "/calc <выражение> — калькулятор (например: /calc 12 * 8)\n"
               "/joke — анекдот про программистов\n"
               "/quote — вдохновляющая цитата\n"
               "/echo <текст> — повторить текст\n"
               "/info — информация о мессенджере";
    }

    if (trimmed.startsWith("/time", Qt::CaseInsensitive)) {
        return "🕒 Сейчас: " + QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm:ss");
    }

    if (trimmed.startsWith("/calc", Qt::CaseInsensitive)) {
        QString expr = trimmed.mid(5).trimmed();
        bool ok = false;
        double res = evaluateSimpleExpr(expr, ok);
        if (ok) {
            return QString("🔢 Результат: %1").arg(res);
        }
        return "❌ Не удалось вычислить. Пример использования: /calc 25 * 4";
    }

    if (trimmed.startsWith("/joke", Qt::CaseInsensitive)) {
        static const QStringList jokes = {
            "Бывают 10 типов людей: те, кто понимают двоичную систему счисления, и те, кто не понимают.",
            "— В чем разница между багом и фичей?\n— Баг — это неожиданная фича, а фича — это запланированный баг.",
            "Программист ставит на тумбочку два стакана: один с водой — если захочет пить, а второй пустой — если не захочет.",
            "Чтобы понять рекурсию, нужно сначала понять рекурсию.",
            "— Папа, а почему солнце встает на востоке и садится на западе?\n— Работает? Ничего не трогай!"
        };
        int idx = QRandomGenerator::global()->bounded(jokes.size());
        return jokes[idx];
    }

    if (trimmed.startsWith("/quote", Qt::CaseInsensitive)) {
        static const QStringList quotes = {
            "«Простота — необходимое условие прекрасного» — Лев Толстой",
            "«Сделай сначала так, чтобы работало, потом правильно, потом быстро» — Кент Бек",
            "«Единственный способ делать великие дела — любить то, что делаешь» — Стив Джобс"
        };
        int idx = QRandomGenerator::global()->bounded(quotes.size());
        return quotes[idx];
    }

    if (trimmed.startsWith("/echo", Qt::CaseInsensitive)) {
        return trimmed.mid(5).trimmed();
    }

    if (trimmed.startsWith("/info", Qt::CaseInsensitive)) {
        return "ℹ️ Telegram Clone Desktop (C++ / Qt 6)\n"
               "Поддерживает обмен сообщениями, закрепление диалогов, поиск, отправку фото, эмодзи и цитирование!";
    }

    const QString lower = trimmed.toLower();
    if (lower.contains("привет") || lower.contains("здравствуй") || lower.contains("хай")) {
        return "Приветствую! Чем могу помочь сегодня? Попробуйте команду /help 🚀";
    }

    if (lower.contains("как дела") || lower.contains("как жизнь")) {
        return "Все системы работают в штатном режиме 🟢 А у вас?";
    }

    if (lower.contains("спасибо")) {
        return "Всегда рад помочь! 👍";
    }

    return QString("Принято: «%1». Напишите /help, чтобы узнать список доступных команд.").arg(trimmed);
}

QString Messenger::BotSimulator::generateDurovResponse(const QString &input) {
    const QString lower = input.toLower();
    if (lower.contains("привет")) {
        return "Привет! Telegram построен на принципах свободы и скорости. Как тебе мессенджер?";
    }
    static const QStringList durovQuotes = {
        "Большинство людей предпочитают комфорт свободе. Но будущее принадлежит тем, кто выбирает независимость.",
        "Приватность не продается, а права человека не должны ставиться под угрозу из-за страха.",
        "Создавать то, что полезно миллионам людей — вот единственное стоящее занятие.",
        "Архитектура и минимализм всегда побеждают перегруженные интерфейсы.",
        "Спасибо за сообщение! Команда продолжает работать над новыми функциями."
    };
    int idx = QRandomGenerator::global()->bounded(durovQuotes.size());
    return durovQuotes[idx];
}

QString Messenger::BotSimulator::generateUserResponse(const QString &input) {
    const QString lower = input.toLower();
    if (lower.contains("привет") || lower.contains("ку")) {
        return "Привет! Как твои дела?";
    }
    if (lower.contains("как дела") || lower.contains("что делаешь")) {
        return "Всё отлично, спасибо! Тестирую новый Telegram клон 🙂";
    }
    if (lower.contains("согласен") || lower.contains("да") || lower.contains("ок")) {
        return "Договорились! 👍";
    }

    static const QStringList genericReplies = {
        "Отлично, понял тебя! 👍",
        "Интересно! Расскажи подробнее.",
        "Круто получилось 🚀",
        "Хорошо, чуть позже гляну!",
        "Супер, на связи!"
    };
    int idx = QRandomGenerator::global()->bounded(genericReplies.size());
    return genericReplies[idx];
}
