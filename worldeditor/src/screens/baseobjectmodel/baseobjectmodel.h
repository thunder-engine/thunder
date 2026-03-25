#ifndef BASEOBJECTMODEL_H
#define BASEOBJECTMODEL_H

#include <QAbstractItemModel>

class BaseObjectModel : public QAbstractItemModel {
    Q_OBJECT

public:
    explicit BaseObjectModel(QObject *parent = nullptr);

    virtual QObject *createRoot();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex &index) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QModelIndex getIndex(QObject *object, const QModelIndex &parent = QModelIndex()) const;

    virtual bool removeResource(const QModelIndex &index);

    void addItem(QObject *object);

    QObject *getObject(const QModelIndex &index) const;

    void clear();

protected:
    QObject *m_rootItem;

    QHash<quintptr, QObject *> m_items;

};

#endif // BASEOBJECTMODEL_H
