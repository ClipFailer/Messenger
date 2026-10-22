#ifndef CHATDELEGATE_H
#define CHATDELEGATE_H

#include <QStyledItemDelegate>

namespace Messenger {

class ChatDelegate : public QStyledItemDelegate {
    Q_OBJECT
  public:
    explicit ChatDelegate(QObject *parent = nullptr);

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void  paint(QPainter *painter, const QStyleOptionViewItem &option,
                const QModelIndex &index) const override;

  private:
    QColor colorForName(const QString &name) const;

    static constexpr int ItemHeight   = 72;
    static constexpr int AvatarSize   = 48;
    static constexpr int AvatarMargin = 12;
};

} // namespace Messenger

#endif // CHATDELEGATE_H
