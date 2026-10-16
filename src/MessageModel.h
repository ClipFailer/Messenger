#ifndef MESSAGEMODEL_H
#define MESSAGEMODEL_H

#include <QAbstractListModel> // ← ListModel, не ItemModel
#include <QStringList>

class MessageModel : public QAbstractListModel {
    Q_OBJECT
  public:
    explicit MessageModel(QObject *parent = nullptr);

    //===== Overrides =====
    int      rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void setMessages(const QStringList &messages);
    void addMessage(const QString &message);
    void clear();

  private:
    QStringList m_messages;
};

#endif // MESSAGEMODEL_H
