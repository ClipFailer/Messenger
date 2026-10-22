#include "chatdelegate.h"
#include "core/ChatModel.h"
#include <QPainter>
#include <QPainterPath>

Messenger::ChatDelegate::ChatDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{}

QSize Messenger::ChatDelegate::sizeHint(const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const {
    Q_UNUSED(index)
    return QSize(option.rect.width(), ItemHeight);
}

QColor Messenger::ChatDelegate::colorForName(const QString &name) const {
    static const QList<QColor> colors = {
        QColor("#e17076"), // Red
        QColor("#faa774"), // Orange
        QColor("#a695e7"), // Violet
        QColor("#7bc862"), // Green
        QColor("#6ec9cb"), // Cyan
        QColor("#65aadd"), // Blue
        QColor("#ee7aae")  // Pink
    };

    uint hash = qHash(name);
    return colors[hash % colors.size()];
}

void Messenger::ChatDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const {
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::TextAntialiasing);

    const QString chatTitle   = index.data(Qt::DisplayRole).toString();
    const QString lastMessage = index.data(Messenger::ChatRoles::LastMessageRole).toString();
    const QString lastTime    = index.data(Messenger::ChatRoles::LastTimeRole).toString();
    const bool    isPinned    = index.data(Messenger::ChatRoles::IsPinnedRole).toBool();
    const QString chatType    = index.data(Messenger::ChatRoles::ChatTypeRole).toString();
    const int     unreadCount = index.data(Messenger::ChatRoles::UnreadCountRole).toInt();
    const QString customColor = index.data(Messenger::ChatRoles::AvatarColorRole).toString();

    const bool isSelected = (option.state & QStyle::State_Selected);
    const bool isHovered  = (option.state & QStyle::State_MouseOver);

    // Фоновая заливка элемента
    QColor bgColor = QColor("#17212b");
    if (isSelected) {
        bgColor = QColor("#2b5278");
    } else if (isHovered) {
        bgColor = QColor("#202b36");
    }
    painter->fillRect(option.rect, bgColor);

    // Отрисовка круглого аватара
    const QRect avatarRect(option.rect.left() + AvatarMargin,
                           option.rect.top() + (ItemHeight - AvatarSize) / 2,
                           AvatarSize, AvatarSize);

    QColor avatarBg = customColor.isEmpty() ? colorForName(chatTitle) : QColor(customColor);
    if (chatType == "saved" || chatTitle == "Избранное") {
        avatarBg = QColor("#2AABEE");
    }

    painter->setPen(Qt::NoPen);
    painter->setBrush(avatarBg);
    painter->drawEllipse(avatarRect);

    // Содержимое аватара (символ или иконка)
    painter->setPen(Qt::white);
    QFont avatarFont = painter->font();
    avatarFont.setBold(true);

    if (chatType == "saved" || chatTitle == "Избранное") {
        avatarFont.setPointSize(16);
        painter->setFont(avatarFont);
        painter->drawText(avatarRect, Qt::AlignCenter, "🔖");
    } else if (chatType == "bot") {
        avatarFont.setPointSize(16);
        painter->setFont(avatarFont);
        painter->drawText(avatarRect, Qt::AlignCenter, "🤖");
    } else {
        avatarFont.setPointSize(13);
        painter->setFont(avatarFont);
        const QString initial = chatTitle.isEmpty() ? "?" : chatTitle.left(1).toUpper();
        painter->drawText(avatarRect, Qt::AlignCenter, initial);
    }

    // Текстовая область
    const int textLeft = option.rect.left() + AvatarMargin * 2 + AvatarSize;
    const int rightMargin = 12;
    const int availableWidth = option.rect.right() - textLeft - rightMargin;

    // Правая часть: время и значок закрепления
    int rightIconsWidth = 0;
    QFont timeFont = painter->font();
    timeFont.setPointSize(9);
    painter->setFont(timeFont);
    const int timeTextWidth = painter->fontMetrics().horizontalAdvance(lastTime);

    if (!lastTime.isEmpty()) {
        rightIconsWidth += timeTextWidth + 6;
    }
    if (isPinned) {
        rightIconsWidth += 18;
    }

    // Заголовок чата
    const int titleWidth = availableWidth - rightIconsWidth;
    const QRect titleRect(textLeft, option.rect.top() + 14, titleWidth, 20);

    QFont titleFont = painter->font();
    titleFont.setBold(true);
    titleFont.setPointSize(10);
    painter->setFont(titleFont);
    painter->setPen(Qt::white);

    const QString elidedTitle = painter->fontMetrics().elidedText(chatTitle, Qt::ElideRight, titleWidth);
    painter->drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter, elidedTitle);

    // Отрисовка времени
    if (!lastTime.isEmpty()) {
        painter->setFont(timeFont);
        painter->setPen(isSelected ? QColor("#c1d8ee") : QColor("#7f91a4"));
        const QRect timeRect(option.rect.right() - rightMargin - timeTextWidth,
                             option.rect.top() + 14, timeTextWidth, 20);
        painter->drawText(timeRect, Qt::AlignRight | Qt::AlignVCenter, lastTime);
    }

    // Вторая строка: последнее сообщение и счетчик непрочитанных / булавка
    const int bottomY = option.rect.top() + 38;
    int badgeWidth = 0;
    if (unreadCount > 0) {
        badgeWidth = 24;
    } else if (isPinned) {
        badgeWidth = 20;
    }

    const int msgWidth = availableWidth - badgeWidth - 4;
    const QRect msgRect(textLeft, bottomY, msgWidth, 20);

    QFont msgFont = painter->font();
    msgFont.setPointSize(9);
    painter->setFont(msgFont);
    painter->setPen(isSelected ? QColor("#dfe8f2") : QColor("#7f91a4"));

    const QString elidedMsg = painter->fontMetrics().elidedText(lastMessage, Qt::ElideRight, msgWidth);
    painter->drawText(msgRect, Qt::AlignLeft | Qt::AlignVCenter, elidedMsg);

    // Отрисовка бейджа непрочитанных или значка закрепления
    if (unreadCount > 0) {
        const int badgeH = 18;
        const int badgeW = qMax(20, painter->fontMetrics().horizontalAdvance(QString::number(unreadCount)) + 10);
        const QRect badgeRect(option.rect.right() - rightMargin - badgeW, bottomY + 1, badgeW, badgeH);

        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor("#4fae63"));
        painter->drawRoundedRect(badgeRect, badgeH / 2, badgeH / 2);

        painter->setPen(Qt::white);
        QFont badgeFont = painter->font();
        badgeFont.setBold(true);
        badgeFont.setPointSize(8);
        painter->setFont(badgeFont);
        painter->drawText(badgeRect, Qt::AlignCenter, QString::number(unreadCount));
    } else if (isPinned) {
        const QRect pinRect(option.rect.right() - rightMargin - 18, bottomY + 1, 18, 18);
        painter->setPen(isSelected ? QColor("#c1d8ee") : QColor("#7f91a4"));
        painter->setFont(timeFont);
        painter->drawText(pinRect, Qt::AlignCenter, "📌");
    }

    painter->restore();
}
