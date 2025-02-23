#include "propertyfilter.h"

PropertyFilter::PropertyFilter(QObject *parent) :
        QSortFilterProxyModel(parent) {
}

void PropertyFilter::setGroup(const QString &group) {
    m_group = group;
    invalidate();
}

void PropertyFilter::sort(int column, Qt::SortOrder order) {
    QSortFilterProxyModel::sort(column, order);
}

bool PropertyFilter::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
    bool result = true;
    if(!m_group.isEmpty()) {
        result = checkGroupFilter(sourceRow, sourceParent);
    }
    result &= checkNameFilter(sourceRow, sourceParent);

    return result;
}

bool PropertyFilter::checkGroupFilter(int sourceRow, const QModelIndex &sourceParent) const {
    if(!sourceParent.isValid()) {
        QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

        QString type = sourceModel()->data(index).toString();
        if(m_group == type || type.isEmpty()) {
            return true;
        }

        return false;
    }

    return true;
}

bool PropertyFilter::checkNameFilter(int sourceRow, const QModelIndex &sourceParent) const {
    QAbstractItemModel *model = sourceModel();
    QModelIndex index = model->index(sourceRow, 0, sourceParent);

    if(!filterRegularExpression().isValid() && index.isValid()) {
        for(int i = 0; i < model->rowCount(index); i++) {
            if(filterAcceptsRow(i, index)) {
                return true;
            }
        }

        QString key = model->data(index, filterRole()).toString();
        return key.contains(filterRegularExpression());
    }
    return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}
