#include "chatdelegate.h"
#include "core/ChatModel.h"
#include <QPainter>

Messenger::ChatDelegate::ChatDelegate(QObject *parent) : QStyledItemDelegate{parent} {}

QSize Messenger::ChatDelegate::sizeHint(const QStyleOptionViewItem &option,
                                        const QModelIndex          &index) const {
    return QSize(option.rect.width(), ItemHeight);
};

void Messenger::ChatDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const {

    // Подготавливаем painter.
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    // Получаем данные из модели.
    const QString chatTitle   = index.data(Qt::DisplayRole).toString();
    const QString lastMessage = index.data(Messenger::ChatRoles::LastMessageRole).toString();

    // Выбираем заливку фона.
    if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, ItemSelectedColor);
    else if (option.state & QStyle::State_MouseOver)
        painter->fillRect(option.rect, ItemHoverColor);

    // Аватарка.

    const QRect avatarRect(option.rect.left() + AvatarMargin,
                           option.rect.top() + (ItemHeight - AvatarSize) / 2, AvatarSize,
                           AvatarSize);

    // Рисуем аватар.
    painter->setPen(Qt::NoPen);
    painter->setBrush(AvatarColor);
    painter->drawEllipse(avatarRect);

    // Рисуем первую букву имени по цетру аватара.
    painter->setPen(AvatarInitilaColor);
    QFont font = painter->font();
    font.setBold(true);
    font.setPointSize(AvatarInitialFontSize);
    painter->setFont(font);

    const QString initial = chatTitle.left(1).toUpper();
    painter->drawText(avatarRect, Qt::AlignCenter, initial);

    // Имя чата.

    const int totalTextHeight = NameHeight + MsgTopMargin + MsgHeight;
    const int textStartY      = option.rect.top() + (ItemHeight - totalTextHeight) / 2;
    const int textLeft        = option.rect.left() + AvatarMargin * 2 + AvatarSize;
    const int textWidth       = option.rect.width() - textLeft - AvatarMargin;

    const QRect nameRect(textLeft, textStartY, textWidth, NameHeight);

    painter->setPen(NameFontColor);
    QFont nameFont = painter->font();
    nameFont.setBold(true);
    nameFont.setPointSize(NameFontSize);
    painter->setFont(nameFont);

    // Отрисовываем название чата.
    painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, chatTitle);

    // Последнее сообщение чата.

    const QRect msgRect(textLeft, textStartY + NameHeight + MsgTopMargin, textWidth, MsgHeight);

    painter->setPen(MsgColor);
    QFont msgFont = painter->font();
    msgFont.setPointSize(MsgFontSize);
    painter->setFont(msgFont);

    // Обрезаем текст, если не влезает.
    const QString elidedMsg =
        painter->fontMetrics().elidedText(lastMessage, Qt::ElideRight, textWidth);

    // Отрисовываем сообщение.
    painter->drawText(msgRect, Qt::AlignLeft | Qt::AlignVCenter, elidedMsg);

    painter->restore();
};