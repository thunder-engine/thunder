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
#include "baseobjectmodel.h"

BaseObjectModel::BaseObjectModel(QObject *parent) :
        QAbstractItemModel(parent) {

    m_rootItem = createRoot();
}

QObject *BaseObjectModel::createRoot() {
    QObject *root = new QObject(this);
    addItem(root);
    return root;
}

int BaseObjectModel::rowCount(const QModelIndex &parent) const {
    QObject *parentItem = m_rootItem;
    if(parent.isValid()) {
        parentItem = getObject(parent);
    }
    if(parentItem) {
        return parentItem->children().size();
    }
    return 0;
}

QModelIndex BaseObjectModel::index(int row, int column, const QModelIndex &parent) const {
    QObject *parentItem = m_rootItem;
    if(parent.isValid()) {
        parentItem = getObject(parent);
    }
    if(!parentItem || row >= parentItem->children().size() || row < 0) {
        return QModelIndex();
    }

    QObject *item = parentItem->children().at(row);
    return createIndex(row, column, reinterpret_cast<quintptr>(item));
}

QModelIndex BaseObjectModel::parent(const QModelIndex &index) const {
    if(!index.isValid()) {
        return QModelIndex();
    }

    QObject *childItem = getObject(index);
    if(childItem) {
        QObject *parentItem = childItem->parent();
        if(!parentItem || parentItem == m_rootItem) {
            return QModelIndex();
        }

        QObject *superParent = parentItem->parent();
        if(!superParent) {
            return QModelIndex();
        }
        int row = superParent->children().indexOf(parentItem);
        return createIndex(row, 0, reinterpret_cast<quintptr>(parentItem));
    }
    return QModelIndex();
}

Qt::ItemFlags BaseObjectModel::flags(const QModelIndex &index) const {
    if(!index.isValid()) {
        return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
    }
    QObject *item = getObject(index);
    // only allow change of value attribute
    if(!item || !item->children().isEmpty()) {
        return Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
    }
    return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex BaseObjectModel::getIndex(QObject *object, const QModelIndex &parent) const {
    for(int i = 0; i < rowCount(parent); i++) {
        QModelIndex index = BaseObjectModel::index(i, 0, parent);
        if(getObject(index) == object) {
            return index;
        }
        index = getIndex(object, index);
        if(index.isValid()) {
            return index;
        }
    }
    return QModelIndex();
}

void BaseObjectModel::addItem(QObject *object) {
    m_items[reinterpret_cast<quintptr>(object)] = object;
}

QObject *BaseObjectModel::getObject(const QModelIndex &index) const {
    quintptr id = index.internalId();
    return m_items.value(id, nullptr);
}

void BaseObjectModel::clear() {
    foreach(QObject *it, m_rootItem->children()) {
        it->setParent(nullptr);
        it->deleteLater();
    }
    m_items.clear();
}

