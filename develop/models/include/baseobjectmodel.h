#ifndef BASEOBJECTMODEL_H
#define BASEOBJECTMODEL_H

#include <QAbstractItemModel>

class BaseObjectModel : public QAbstractItemModel {
    Q_OBJECT

public:
    explicit BaseObjectModel    (QObject *parent = 0);

    virtual QObject            *createRoot                  ();

    int                         rowCount                    (const QModelIndex &parent = QModelIndex()) const;

    QModelIndex                 parent                      (const QModelIndex &index) const;

    QModelIndex                 index                       (int row, int column, const QModelIndex &parent = QModelIndex()) const;

    Qt::ItemFlags               flags                       (const QModelIndex &index) const;

protected:
    QObject                    *m_rootItem;

};

#endif // BASEOBJECTMODEL_H
