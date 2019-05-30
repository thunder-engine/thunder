#include "assetlist.h"

#include "config.h"

#include <engine.h>

#include "assetmanager.h"

#include <QUuid>

const QString gIcon("icon");
const QString gUuid("uuid");

AssetList *AssetList::m_pInstance   = nullptr;

AssetList::AssetList() :
        BaseObjectModel(nullptr) {

    m_DefaultIcon = QRect(0, 0, 64, 64);
}

AssetList *AssetList::instance() {
    if(!m_pInstance) {
        m_pInstance = new AssetList;
    }
    return m_pInstance;
}

void AssetList::destroy() {
    delete m_pInstance;
    m_pInstance = nullptr;
}

void AssetList::init(Engine *engine) {
    m_pEngine = engine;
}

int AssetList::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return 3;
}

QVariant AssetList::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const {
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 1: return tr("Uuid");
            case 2: return tr("Type");
            default: return tr("Name");
        }
    }
    return QVariant();
}

QVariant AssetList::data(const QModelIndex &index, int role) const {
    if(!index.isValid()) {
        return QVariant();
    }

    QObject *item = static_cast<QObject *>(index.internalPointer());
    QFileInfo info(item->objectName());
    switch(role) {
        case Qt::DisplayRole: {
            switch(index.column()) {
                case 1:  return item->objectName();
                case 2:  return item->property(qPrintable(gType));
                default: return info.baseName();
            }
        }
        case Qt::SizeHintRole: {
            return QSize(m_DefaultIcon.width() + 16, m_DefaultIcon.height() + 16);
        }
        case Qt::DecorationRole: {
            return item->property(qPrintable(gIcon)).value<QImage>();
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

void AssetList::update() {
    foreach(QObject *it, m_rootItem->children()) {
        it->setParent(nullptr);
        delete it;
    }

    AssetManager *inst = AssetManager::instance();
    for(auto it : m_pEngine->indices()) {
        QObject *item   = new QObject(m_rootItem);
        QFileInfo info(it.first.c_str());

        item->setObjectName(info.filePath());
        item->setProperty(qPrintable(gUuid), it.second.c_str());
        int32_t type = inst->assetType(it.second.c_str());
        item->setProperty(qPrintable(gType), type);

        QImage img = inst->icon(it.first.c_str());
        if(!img.isNull()) {
            img = (img.height() < img.width()) ? img.scaledToWidth(m_DefaultIcon.width()) : img.scaledToHeight(m_DefaultIcon.height());
        }
        item->setProperty(qPrintable(gIcon), img);
    }

    emit layoutAboutToBeChanged();
    emit layoutChanged();
}

QImage AssetList::icon(const QModelIndex &index) const {
    QObject *item   = static_cast<QObject *>(index.internalPointer());
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
    QObject *item   = m_rootItem->findChild<QObject *>(resource);
    if(item) {
        return createIndex(item->parent()->children().indexOf(item), 0, item);
    }
    return QModelIndex();
}
