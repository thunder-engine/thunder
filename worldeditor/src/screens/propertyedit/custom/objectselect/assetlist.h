#ifndef ASSETLIST_H
#define ASSETLIST_H

#include "screens/baseobjectmodel/baseobjectmodel.h"

#include <QRect>
#include <QSortFilterProxyModel>

class AssetList  : public BaseObjectModel {
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

class AssetFilter : public QSortFilterProxyModel {
public:
    typedef QList<int32_t> TypeList;

    explicit AssetFilter(QObject *parent) :
            QSortFilterProxyModel(parent) {
        sort(0);
    }

    void setContentType(const QString &type) {
        m_type = type;
        invalidate();
    }

protected:
    bool checkContentTypeFilter(int sourceRow, const QModelIndex &sourceParent) const {
        QModelIndex index = sourceModel()->index(sourceRow, 2, sourceParent);
        QString type = sourceModel()->data(index).toString();
        if(type.isEmpty() || m_type == type) {
            return true;
        }
        return false;
    }

    bool checkNameFilter(int sourceRow, const QModelIndex &sourceParent) const {
        QModelIndex index = sourceModel()->index(sourceRow, 2, sourceParent);
        return(QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent) &&
                                                       (filterRegExp().isEmpty() || sourceModel()->data(index).toBool()));
    }

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override {
        bool result = true;
        if(!m_type.isEmpty()) {
            result = checkContentTypeFilter(sourceRow, sourceParent);
        }
        result &= checkNameFilter(sourceRow, sourceParent);

        return result;
    }

private:
    QString m_type;

};

#endif // ASSETLIST_H
