#include "assetlist.h"

#include "config.h"

#include <engine.h>
#include <systems/resourcesystem.h>

#include <editor/assetmanager.h>

namespace {
    const char *gUuid("uuid");
    const char *gName("name");
};

AssetList::AssetList() :
        BaseObjectModel(nullptr),
        m_defaultIcon(QRect(0, 0, 64, 64)) {

    connect(AssetManager::instance(), &AssetManager::importFinished, this, &AssetList::update);
    connect(AssetManager::instance(), &AssetManager::iconUpdated, this, &AssetList::onRendered);

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

    QObject *item = static_cast<QObject *>(index.internalPointer());
    switch(role) {
        case Qt::DisplayRole: {
            switch(index.column()) {
                case 1:  return item->objectName();
                case 2:  return item->property(gType);
                default: return item->property(gName);
            }
        }
        case Qt::SizeHintRole: {
            return QSize(m_defaultIcon.width() + 16, m_defaultIcon.height() + 16);
        }
        case Qt::DecorationRole: {
            return item->property(gIcon).value<QImage>();
        }
        case Qt::ToolTipRole: {
            return item->objectName();
        }
        default: break;
    }
    return QVariant();
}

Qt::ItemFlags AssetList::flags(const QModelIndex &index) const {
    Qt::ItemFlags result = BaseObjectModel::flags(index);
    result |= Qt::ItemIsSelectable;
    return result;
}

void AssetList::onRendered(const TString &uuid) {
    AssetManager *mgr = AssetManager::instance();
    TString path = mgr->uuidToPath(uuid);
    QObject *item = m_rootItem->findChild<QObject *>(path.data());
    if(item) {
        item->setProperty(gType, mgr->assetTypeName(path).data());

        QImage img = mgr->icon(path);
        if(!img.isNull()) {
            item->setProperty(gIcon, (img.height() < img.width()) ? img.scaledToWidth(m_defaultIcon.width()) : img.scaledToHeight(m_defaultIcon.height()));
        }

        emit layoutAboutToBeChanged();
        emit layoutChanged();
    }
}

void AssetList::update() {
    foreach(QObject *it, m_rootItem->children()) {
        it->setParent(nullptr);
        delete it;
    }

    AssetManager *inst = AssetManager::instance();
    for(auto &it : Engine::resourceSystem()->indices()) {
        QObject *item = new QObject(m_rootItem);

        TString path = inst->uuidToPath(it.second.uuid);
        item->setObjectName(path.data());
        item->setProperty(gUuid, it.second.uuid.data());
        item->setProperty(gType, it.second.type.data());
        item->setProperty(gName, Url(path).baseName().data());

        QImage img = inst->icon(path);
        if(!img.isNull()) {
            img = (img.height() < img.width()) ? img.scaledToWidth(m_defaultIcon.width()) : img.scaledToHeight(m_defaultIcon.height());
        }
        item->setProperty(qPrintable(gIcon), img);
    }

    emit layoutAboutToBeChanged();
    emit layoutChanged();
}

QImage AssetList::icon(const QModelIndex &index) const {
    QObject *item = static_cast<QObject *>(index.internalPointer());
    if(item) {
        return item->property(qPrintable(gIcon)).value<QImage>();
    }
    return QImage();
}

QString AssetList::path(const QModelIndex &index) const {
    QObject *item = static_cast<QObject *>(index.internalPointer());
    if(item) {
        return item->objectName();
    }
    return QString();
}

QModelIndex AssetList::findResource(const QString &resource) const {
    QObject *item = m_rootItem->findChild<QObject *>(resource);
    if(item) {
        return createIndex(item->parent()->children().indexOf(item), 0, item);
    }
    return QModelIndex();
}
