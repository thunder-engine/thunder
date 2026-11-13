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
#include "componentmodel.h"

#include <engine.h>
#include <url.h>

#include <editor/assetmanager.h>

const char *gURI("uri");

class ComponentModelProxy : public Object {
    A_OBJECT(ComponentModelProxy, Object, Editor)

    A_METHODS(
        A_SLOT(ComponentModelProxy::onBuildSuccessful)
    )

    void setOrigin(ComponentModel *origin) {
        m_origin = origin;

        connect(Editor::assets(), _SIGNAL(buildSuccessful(bool)), this, _SLOT(onBuildSuccessful(bool)));
    }

    void onBuildSuccessful(bool flag) {
        m_origin->update();
    }

private:
    ComponentModel *m_origin = nullptr;
};

ComponentModel::ComponentModel() :
        BaseObjectModel(nullptr),
        m_componentModelProxy(new ComponentModelProxy) {

    m_componentModelProxy->setOrigin(this);
}

ComponentModel *ComponentModel::instance() {
    static ComponentModel instance;
    return &instance;
}

int ComponentModel::columnCount(const QModelIndex &) const {
    return 3;
}

QVariant ComponentModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const {
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch(section) {
            case 0: return tr("Name");
        }
    }
    return QVariant();
}

QVariant ComponentModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid()) {
        return QVariant();
    }
    QObject *item = getObject(index);

    switch(role) {
        case Qt::ToolTipRole:
        case Qt::DisplayRole: {
            switch(index.column()) {
                case 0: return item->objectName();
                case 1: return item->property(gURI).toString();
                case 2: return item->children().empty();
                default: break;
            }
        }
        default: break;
    }
    return QVariant();
}

void ComponentModel::update() {
    clear();

    // Iterate all components
    for(const auto &it : ObjectSystem::factories()) {
        Url url(it.second);

        QObject *item = m_rootItem;
        StringList list = url.filePath().split("/");
        int i = 0;
        foreach(const auto &part, list) {
            QObject *p = item;
            item = nullptr;
            foreach(QObject *it, p->children()) {
                if(part == it->objectName().toStdString()) {
                    item = it;
                    break;
                }
            }
            if(!item) {
                item = new QObject(p);
                item->setObjectName(part.data());
                item->setProperty(gURI, it.second.data());
                addItem(item);
            }
            i++;
        }
    }

    emit layoutAboutToBeChanged();
    emit layoutChanged();
}
