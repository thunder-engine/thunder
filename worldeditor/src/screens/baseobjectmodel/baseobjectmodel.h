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

    void addItem(QObject *object);

    QObject *getObject(const QModelIndex &index) const;

    void clear();

protected:
    QObject *m_rootItem;

    QHash<quintptr, QObject *> m_items;

};

#endif // BASEOBJECTMODEL_H
