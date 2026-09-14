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
#include "assetlist.h"

#include "config.h"

#include <url.h>

#include <engine.h>
#include <systems/resourcesystem.h>

#include <editor/assetmanager.h>
#include <editor/assetconverter.h>
#include <editor/projectsettings.h>

namespace {
    const char *gUuid("uuid");
    const char *gName("name");
};

AssetList::AssetList() :
        BaseObjectModel(nullptr),
        m_cellSzie(64, 64) {

    connect(Editor::assets(), &AssetManager::importFinished, this, &AssetList::update);
    connect(Editor::assets(), &AssetManager::iconUpdated, this, &AssetList::onRendered);

    update();
}

int AssetList::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return 3;
}

QVariant AssetList::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const {
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 1: return "Uuid";
            case 2: return "Type";
            default: return "Name";
        }
    }
    return QVariant();
}

QVariant AssetList::data(const QModelIndex &index, int role) const {
    if(!index.isValid()) {
        return QVariant();
    }

    QObject *item = getObject(index);
    if(item) {
        switch(role) {
            case Qt::DisplayRole: {
                switch(index.column()) {
                    case 1:  return item->objectName();
                    case 2:  return item->property(gType);
                    default: return item->property(gName);
                }
            }
            case Qt::SizeHintRole: return QSize(m_cellSzie.width() + 16, m_cellSzie.height() + 16);
            case Qt::DecorationRole: return item->property(gIcon).value<QImage>();
            case Qt::ToolTipRole: return item->objectName();
            default: break;
        }
    }
    return QVariant();
}

Qt::ItemFlags AssetList::flags(const QModelIndex &index) const {
    Qt::ItemFlags result = BaseObjectModel::flags(index);
    result |= Qt::ItemIsSelectable;
    return result;
}

void AssetList::onRendered(const TString &uuid) {
    AssetManager *mgr = Editor::assets();
    TString path = mgr->uuidToPath(uuid);
    QObject *item = m_rootItem->findChild<QObject *>(path.data());
    if(item) {
        item->setProperty(gType, mgr->assetTypeName(path).data());
        AssetConverterSettings *settings = mgr->fetchSettings(path);
        if(settings) {
            QImage img = settings->icon(uuid);
            if(!img.isNull()) {
                item->setProperty(gIcon, (img.height() < img.width()) ? img.scaledToWidth(m_cellSzie.width()) : img.scaledToHeight(m_cellSzie.height()));
            }
        }

        emit layoutAboutToBeChanged();
        emit layoutChanged();
    }
}

void AssetList::update() {
    clear();

    AssetManager *mgr = Editor::assets();
    for(auto &it : Engine::resourceSystem()->indices()) {
        QObject *item = new QObject(m_rootItem);

        TString path = mgr->uuidToPath(it.second.uuid);
        item->setObjectName(path.data());
        item->setProperty(gUuid, it.second.uuid.data());
        item->setProperty(gType, it.second.type.data());
        item->setProperty(gName, Url(path).baseName().data());

        AssetConverterSettings *settings = mgr->fetchSettings(path);
        if(settings) {
            QImage img = settings->icon(it.second.uuid);
            if(!img.isNull()) {
                img = (img.height() < img.width()) ? img.scaledToWidth(m_cellSzie.width()) : img.scaledToHeight(m_cellSzie.height());
            }
            item->setProperty(gIcon, img);
        }
        addItem(item);
    }

    emit layoutAboutToBeChanged();
    emit layoutChanged();
}

QImage AssetList::icon(const QModelIndex &index) const {
    QObject *item = getObject(index);
    if(item) {
        return item->property(gIcon).value<QImage>();
    }
    return QImage();
}

QString AssetList::path(const QModelIndex &index) const {
    QObject *item = getObject(index);
    if(item) {
        return item->objectName();
    }
    return QString();
}

QModelIndex AssetList::findResource(const QString &resource) const {
    QObject *item = m_rootItem->findChild<QObject *>(resource);
    if(item) {
        return createIndex(item->parent()->children().indexOf(item), 0, reinterpret_cast<quintptr>(item));
    }
    return QModelIndex();
}
