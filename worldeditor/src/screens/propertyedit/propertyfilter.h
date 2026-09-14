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
