#ifndef PROPERTYFILTER_H
#define PROPERTYFILTER_H

#include <QSortFilterProxyModel>

class PropertyFilter : public QSortFilterProxyModel {
public:
    explicit PropertyFilter(QObject *parent);

    void setGroup(const QString &group);

protected:
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

    bool checkGroupFilter(int sourceRow, const QModelIndex &sourceParent) const;

    bool checkNameFilter(int sourceRow, const QModelIndex &sourceParent) const;

protected:
    QString m_group;

};

#endif // PROPERTYFILTER_H
