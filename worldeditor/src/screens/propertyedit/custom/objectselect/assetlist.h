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
#ifndef ASSETLIST_H
#define ASSETLIST_H

#include "screens/baseobjectmodel/baseobjectmodel.h"

#include <QRect>
#include <QSortFilterProxyModel>

class TString;

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
    void onRendered(const TString &uuid);

    void update();

private:
    QSize m_cellSzie;

};

#endif // ASSETLIST_H
