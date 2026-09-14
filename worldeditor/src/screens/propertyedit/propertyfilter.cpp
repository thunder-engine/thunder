/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
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

#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    QRegularExpression reg = filterRegularExpression();
#else
    QRegExp reg = filterRegExp();
#endif

    if(reg.isValid() && index.isValid()) {
        for(int i = 0; i < model->rowCount(index); i++) {
            if(checkNameFilter(i, index)) {
                return true;
            }
        }

        QString key = model->data(index, filterRole()).toString();
        return key.contains(reg);
    }
    return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}
