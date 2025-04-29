#ifndef ASSETLIST_H
#define ASSETLIST_H

#include "screens/baseobjectmodel/baseobjectmodel.h"

#include <QRect>
#include <QSortFilterProxyModel>

class AssetList : public BaseObjectModel {
    Q_OBJECT

public:
    AssetList();

    int columnCount(const QModelIndex &parent) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const;

    QVariant data(const QModelIndex &index, int role) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;

    QImage icon(const QModelIndex &index) const;

    QString path(const QModelIndex &index) const;

    QModelIndex findResource(const QString &resource) const;

public slots:
    void onRendered(const QString &uuid);

    void update();

private:
    QRect m_defaultIcon;

};

#endif // ASSETLIST_H
