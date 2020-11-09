#include "assetlist.h"

#include "config.h"

#include <engine.h>
#include <systems/resourcesystem.h>

#include "assetmanager.h"
#include "projectmanager.h"

#include <QUuid>

const QString gUuid("uuid");

AssetList *AssetList::m_pInstance = nullptr;

AssetList::AssetList() :
        BaseObjectModel(nullptr),
        m_pEngine(nullptr) {

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

void AssetList::onRendered(const QString &uuid) {
    AssetManager *mgr = AssetManager::instance();
    QString path = mgr->guidToPath(uuid.toStdString()).c_str();
    QObject *item = m_rootItem->findChild<QObject *>(path);
    if(item) {
        item->setProperty(qPrintable(gType), mgr->assetTypeName(path));

        QImage img = mgr->icon(path);
        if(!img.isNull()) {
            item->setProperty(qPrintable(gIcon), (img.height() < img.width()) ? img.scaledToWidth(m_DefaultIcon.width()) : img.scaledToHeight(m_DefaultIcon.height()));
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
    for(auto it : static_cast<ResourceSystem *>(m_pEngine->resourceSystem())->indices()) {
        QObject *item = new QObject(m_rootItem);
        QFileInfo info(it.first.c_str());

        QString path = inst->guidToPath(it.second.second).c_str();
        item->setObjectName(path/*info.filePath()*/);
        item->setProperty(qPrintable(gUuid), it.second.second.c_str());

        QString type = inst->assetTypeName(path);
        item->setProperty(qPrintable(gType), type);

        QImage img = inst->icon(path);
        if(!img.isNull()) {
            img = (img.height() < img.width()) ? img.scaledToWidth(m_DefaultIcon.width()) : img.scaledToHeight(m_DefaultIcon.height());
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
