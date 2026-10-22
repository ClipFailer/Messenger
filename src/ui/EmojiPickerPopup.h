#ifndef EMOJIPICKERPOPUP_H
#define EMOJIPICKERPOPUP_H

#include <QWidget>

namespace Messenger {

class EmojiPickerPopup : public QWidget {
    Q_OBJECT
  public:
    explicit EmojiPickerPopup(QWidget *parent = nullptr);

  signals:
    void emojiSelected(const QString &emoji);

  protected:
    void hideEvent(QHideEvent *event) override;
};

} // namespace Messenger

#endif // EMOJIPICKERPOPUP_H
