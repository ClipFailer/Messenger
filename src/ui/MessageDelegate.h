#ifndef MESSAGEDELEGATE_H
#define MESSAGEDELEGATE_H

#include <QStyledItemDelegate>

namespace Messenger {

class MessageDelegate : public QStyledItemDelegate {
    Q_OBJECT
  public:
    explicit MessageDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

  private:
    QRect calculateBubbleGeometry(const QStyleOptionViewItem &option, const QModelIndex &index,
                                  int &outTextHeight, int &outImageHeight, int &outReplyHeight) const;
};

} // namespace Messenger

#endif // MESSAGEDELEGATE_H
