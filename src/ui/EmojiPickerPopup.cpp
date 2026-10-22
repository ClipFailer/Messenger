#include "EmojiPickerPopup.h"
#include <QGridLayout>
#include <QPushButton>
#include <QScrollArea>

Messenger::EmojiPickerPopup::EmojiPickerPopup(QWidget *parent)
    : QWidget(parent, Qt::Popup | Qt::FramelessWindowHint)
{
    setAttribute(Qt::WA_DeleteOnClose, false);
    setObjectName("emojiPickerPopup");
    setFixedSize(290, 220);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("background-color: transparent; border: none;");

    auto *container = new QWidget();
    container->setStyleSheet("background-color: transparent;");
    auto *grid = new QGridLayout(container);
    grid->setSpacing(4);
    grid->setContentsMargins(2, 2, 2, 2);

    const QStringList emojis = {
        "👍", "👎", "❤️", "🔥", "🎉", "👏", "🚀", "💡",
        "😀", "😂", "🤣", "😊", "😍", "😎", "🥳", "🤔",
        "🤫", "😴", "😭", "🤯", "🤖", "⚡", "✨", "💯",
        "👋", "🤝", "👌", "✌️", "💪", "🍕", "☕", "💻"
    };

    int row = 0;
    int col = 0;
    const int cols = 6;

    for (const QString &emo : emojis) {
        auto *btn = new QPushButton(emo, container);
        btn->setObjectName("emojiBtnItem");
        btn->setFixedSize(38, 38);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet("QPushButton { font-size: 18px; padding: 0px; margin: 0px; background-color: transparent; border: none; border-radius: 6px; } "
                           "QPushButton:hover { background-color: #2b5278; }");
        connect(btn, &QPushButton::clicked, this, [this, emo]() {
            emit emojiSelected(emo);
            close();
        });
        grid->addWidget(btn, row, col);

        col++;
        if (col >= cols) {
            col = 0;
            row++;
        }
    }

    scrollArea->setWidget(container);
    mainLayout->addWidget(scrollArea);

    setStyleSheet("QWidget#emojiPickerPopup { background-color: #17212b; border: 1px solid #242f3d; border-radius: 10px; }");
}

void Messenger::EmojiPickerPopup::hideEvent(QHideEvent *event) {
    QWidget::hideEvent(event);
}
