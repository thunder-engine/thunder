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

    virtual bool removeResource(const QModelIndex &index) { Q_UNUSED(index); return false; }

    virtual QString path(const QModelIndex &index) const { Q_UNUSED(index); return QString(); }

    QModelIndex getIndex(QObject *object, const QModelIndex &parent = QModelIndex()) const;

protected:
    QObject *m_rootItem;

};

#endif // BASEOBJECTMODEL_H
