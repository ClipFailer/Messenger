#ifndef CHATDELEGATE_H
#define CHATDELEGATE_H

#include <QStyledItemDelegate>

namespace Messenger {
class ChatDelegate : public QStyledItemDelegate {
    Q_OBJECT
  public:
    explicit ChatDelegate(QObject *parent = nullptr);

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

  private:
    static constexpr int       ItemHeight        = 72;
    inline static const QColor ItemSelectedColor = QColor(43, 45, 48); // Тёмно-серый фон
    inline static const QColor ItemHoverColor    = QColor(32, 34, 36); // Фон при наведении

    static constexpr int       AvatarSize            = 48;
    static constexpr int       AvatarMargin          = 12;
    static constexpr int       AvatarInitialFontSize = 12;
    inline static const QColor AvatarInitilaColor    = QColor(255, 255, 255); // Белый
    inline static const QColor AvatarColor           = QColor(0, 122, 255);   // Синий

    static constexpr int       NameTopMargin = 12;
    static constexpr int       NameHeight    = 20;
    static constexpr int       NameFontSize  = 11;
    inline static const QColor NameFontColor = QColor(255, 255, 255); // Белый

    static constexpr int       MsgTopMargin = 2;
    static constexpr int       MsgHeight    = 15;
    static constexpr int       MsgFontSize  = 10;
    inline static const QColor MsgColor     = QColor(136, 136, 136); // Серый
};
} // namespace Messenger

#endif // CHATDELEGATE_H
