#include "MessageDelegate.h"

#include "core/MessageModel.h"
#include <QPainter>

Messenger::MessageDelegate::MessageDelegate(QObject *parent) : QStyledItemDelegate{parent} {}

void Messenger::MessageDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const {
    // Сохраняем состояние.
    painter->save();

    // Сглаживание.
    painter->setRenderHint(QPainter::Antialiasing);

    // Получаем текст сообщения.
    const QString messageText = index.data(Qt::DisplayRole).toString();

    // Определяем, наше сообщение или собеседника.
    const bool isOutgoing = index.data(Messenger::Roles::IsOutgoingRole).toBool();

    // Размер пузыря.
    const int maxWidth = option.rect.width() * 0.7;

    const QRect textRect =
        option.fontMetrics.boundingRect(QRect(0, 0, maxWidth, 0), Qt::TextWordWrap, messageText);

    // Отступы.
    const int padding = 8;
    const int margin  = 16;

    // Размеры.
    const int width  = textRect.width() + padding * 2;
    const int height = textRect.height() + padding * 2;

    // Позиция.
    const int x = isOutgoing ? option.rect.right() - width - margin : option.rect.left() + margin;
    const int y = option.rect.top() + margin;

    QRect bubbleRect(x, y, width, height);

    painter->setPen(Qt::NoPen);

    // Цвет баблац
    const QColor bubbleColor = isOutgoing ? QColor("#007aff") : QColor("#e5e5ea");
    painter->setBrush(bubbleColor);
    painter->drawRoundedRect(bubbleRect, 10, 10);

    // Цвет текста.
    const QColor textColor = isOutgoing ? Qt::white : Qt::black;
    painter->setPen(textColor);

    // Прямоугольник для текста.
    QRect textDrawRect = bubbleRect.adjusted(padding, padding, -padding, -padding);
    painter->drawText(textDrawRect, Qt::TextWordWrap, messageText);

    painter->restore();
}

QSize Messenger::MessageDelegate::sizeHint(const QStyleOptionViewItem &option,
                                           const QModelIndex          &index) const {

    // Получаем текст сообщения.
    QString messageText = index.data(Qt::DisplayRole).toString();

    // оступы.
    const int padding = 8;
    const int margin  = 8;

    // Определяем ширину.
    const int maxWidth = option.rect.width() * 0.7;
    QRect     textRect =
        option.fontMetrics.boundingRect(QRect(0, 0, maxWidth, 0), Qt::TextWordWrap, messageText);

    int totalHeight = textRect.height() + (padding * 2) + (margin * 2);

    return QSize(option.rect.width(), totalHeight);
}