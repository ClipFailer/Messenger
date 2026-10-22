#include "MessageDelegate.h"
#include "core/MessageModel.h"
#include <QFileInfo>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>

namespace {
constexpr int BubblePadding = 10;
constexpr int BubbleMargin = 12;
constexpr int ItemMargin = 4;
constexpr int MaxImageWidth = 240;
constexpr int MaxImageHeight = 180;
} // namespace

Messenger::MessageDelegate::MessageDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{}

QRect Messenger::MessageDelegate::calculateBubbleGeometry(const QStyleOptionViewItem &option,
                                                          const QModelIndex &index,
                                                          int &outTextHeight, int &outImageHeight,
                                                          int &outReplyHeight) const {
    const QString text      = index.data(Qt::DisplayRole).toString();
    const QString filePath  = index.data(Messenger::Roles::FilePathRole).toString();
    const QString replyText = index.data(Messenger::Roles::ReplyTextRole).toString();
    const QString timestamp = index.data(Messenger::Roles::TimestampRole).toString();
    const bool    isEdited  = index.data(Messenger::Roles::IsEditedRole).toBool();
    const bool    isOutgoing = index.data(Messenger::Roles::IsOutgoingRole).toBool();

    const int maxBubbleWidth = qMax(220, static_cast<int>(option.rect.width() * 0.65));
    const int contentWidth   = maxBubbleWidth - BubblePadding * 2;

    outReplyHeight = 0;
    if (!replyText.isEmpty()) {
        outReplyHeight = 36;
    }

    outImageHeight = 0;
    int imageWidth = 0;
    if (!filePath.isEmpty() && QFileInfo::exists(filePath)) {
        QPixmap pix(filePath);
        if (!pix.isNull()) {
            QSize scaled = pix.size().scaled(MaxImageWidth, MaxImageHeight, Qt::KeepAspectRatio);
            imageWidth = scaled.width();
            outImageHeight = scaled.height() + 6;
        }
    }

    outTextHeight = 0;
    int textWidth = 0;
    if (!text.isEmpty()) {
        QFont textFont = option.font;
        textFont.setPointSize(10);
        QFontMetrics fm(textFont);
        QRect textRect = fm.boundingRect(QRect(0, 0, contentWidth, 0), Qt::TextWordWrap, text);
        outTextHeight = textRect.height();
        textWidth = textRect.width();
    }

    // Метаданные (время + галочки)
    QFont timeFont = option.font;
    timeFont.setPointSize(8);
    QFontMetrics tfm(timeFont);
    QString metaStr = timestamp;
    if (isEdited) metaStr = "(изм.) " + metaStr;
    if (isOutgoing) metaStr += " ✓✓";
    int metaWidth = tfm.horizontalAdvance(metaStr) + 8;

    int bubbleWidth = qMax(imageWidth, qMax(textWidth, metaWidth)) + BubblePadding * 2;
    if (!replyText.isEmpty()) {
        bubbleWidth = qMax(bubbleWidth, qMin(240, maxBubbleWidth));
    }
    bubbleWidth = qMin(bubbleWidth, maxBubbleWidth);
    bubbleWidth = qMax(bubbleWidth, 120);

    const int timeRowHeight = 14;
    int bubbleHeight = BubblePadding * 2 + outReplyHeight + outImageHeight + outTextHeight + timeRowHeight;

    int x = isOutgoing ? (option.rect.right() - bubbleWidth - BubbleMargin)
                       : (option.rect.left() + BubbleMargin);
    int y = option.rect.top() + ItemMargin;

    return QRect(x, y, bubbleWidth, bubbleHeight);
}

void Messenger::MessageDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const {
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->setRenderHint(QPainter::TextAntialiasing);

    const QString text       = index.data(Qt::DisplayRole).toString();
    const bool    isOutgoing = index.data(Messenger::Roles::IsOutgoingRole).toBool();
    const QString timestamp  = index.data(Messenger::Roles::TimestampRole).toString();
    const QString filePath   = index.data(Messenger::Roles::FilePathRole).toString();
    const QString replyText  = index.data(Messenger::Roles::ReplyTextRole).toString();
    const bool    isEdited   = index.data(Messenger::Roles::IsEditedRole).toBool();

    int textHeight = 0;
    int imageHeight = 0;
    int replyHeight = 0;
    QRect bubbleRect = calculateBubbleGeometry(option, index, textHeight, imageHeight, replyHeight);

    // Цвет фона сообщения
    const QColor bubbleColor = isOutgoing ? QColor("#2b5278") : QColor("#182533");
    painter->setPen(Qt::NoPen);
    painter->setBrush(bubbleColor);
    painter->drawRoundedRect(bubbleRect, 12, 12);

    int curY = bubbleRect.top() + BubblePadding;
    const int contentX = bubbleRect.left() + BubblePadding;
    const int contentW = bubbleRect.width() - BubblePadding * 2;

    // Отрисовка цитируемого ответа
    if (!replyText.isEmpty()) {
        const QRect replyRect(contentX, curY, contentW, 30);
        painter->setPen(Qt::NoPen);
        painter->setBrush(isOutgoing ? QColor("#224160") : QColor("#121b24"));
        painter->drawRoundedRect(replyRect, 4, 4);

        // Полоса цитаты
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor("#5494d4"));
        painter->drawRect(QRect(contentX, curY, 3, 30));

        // Текст цитаты
        QFont replyFont = painter->font();
        replyFont.setPointSize(8);
        replyFont.setBold(true);
        painter->setFont(replyFont);
        painter->setPen(QColor("#65aadd"));
        painter->drawText(QRect(contentX + 8, curY + 2, contentW - 12, 13), Qt::AlignLeft | Qt::AlignVCenter, "В ответ на:");

        replyFont.setBold(false);
        painter->setFont(replyFont);
        painter->setPen(QColor("#cfdbe6"));
        QString elidedReply = painter->fontMetrics().elidedText(replyText, Qt::ElideRight, contentW - 12);
        painter->drawText(QRect(contentX + 8, curY + 15, contentW - 12, 13), Qt::AlignLeft | Qt::AlignVCenter, elidedReply);

        curY += replyHeight;
    }

    // Отрисовка прикрепленного фото
    if (!filePath.isEmpty() && QFileInfo::exists(filePath)) {
        QPixmap pix(filePath);
        if (!pix.isNull()) {
            QSize scaledSize = pix.size().scaled(MaxImageWidth, MaxImageHeight, Qt::KeepAspectRatio);
            QRect imgRect(contentX, curY, scaledSize.width(), scaledSize.height());

            QPainterPath clipPath;
            clipPath.addRoundedRect(imgRect, 8, 8);
            painter->save();
            painter->setClipPath(clipPath);
            painter->drawPixmap(imgRect, pix);
            painter->restore();

            curY += imageHeight;
        }
    }

    // Отрисовка текста сообщения
    if (!text.isEmpty()) {
        QFont textFont = painter->font();
        textFont.setPointSize(10);
        painter->setFont(textFont);
        painter->setPen(Qt::white);

        QRect textDrawRect(contentX, curY, contentW, textHeight);
        painter->drawText(textDrawRect, Qt::TextWordWrap, text);
        curY += textHeight;
    }

    // Отрисовка времени и статуса (внизу справа)
    QFont timeFont = painter->font();
    timeFont.setPointSize(8);
    painter->setFont(timeFont);

    QString metaStr = timestamp;
    if (isEdited) metaStr = "(изм.) " + metaStr;
    if (isOutgoing) metaStr += " ✓✓";

    painter->setPen(isOutgoing ? QColor("#8ab6df") : QColor("#708499"));
    const QRect metaRect(contentX, bubbleRect.bottom() - BubblePadding - 10, contentW, 14);
    painter->drawText(metaRect, Qt::AlignRight | Qt::AlignVCenter, metaStr);

    painter->restore();
}

QSize Messenger::MessageDelegate::sizeHint(const QStyleOptionViewItem &option,
                                           const QModelIndex &index) const {
    int textHeight = 0;
    int imageHeight = 0;
    int replyHeight = 0;
    QRect bubbleRect = calculateBubbleGeometry(option, index, textHeight, imageHeight, replyHeight);
    return QSize(option.rect.width(), bubbleRect.height() + ItemMargin * 2);
}
