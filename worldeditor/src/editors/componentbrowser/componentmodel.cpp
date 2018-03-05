#include "componentmodel.h"

#include <QUrl>

#include <engine.h>

#define URI "uri"

ComponentModel::ComponentModel() :
        BaseObjectModel(nullptr),
        m_pEngine(nullptr) {
}

void ComponentModel::init(Engine *engine) {
    m_pEngine   = engine;

    update();
}

int ComponentModel::columnCount(const QModelIndex &parent) const {
    return 3;
}

QVariant ComponentModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const {
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0: return tr("Name");
        }
    }
    return QVariant();
}

QVariant ComponentModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid()) {
        return QVariant();
    }
    QObject *item   = static_cast<QObject* >(index.internalPointer());

    switch(role) {
        case Qt::ToolTipRole:
        case Qt::DisplayRole: {
            switch(index.column()) {
                case 0: return item->objectName();
                case 1: return item->property(URI).toString();
                case 2: return item->children().empty();
                default: break;
            }
        }
        default: break;
    }
    return QVariant();
}

void ComponentModel::update() {
    foreach(QObject *it, m_rootItem->children()) {
        it->setParent(nullptr);
        it->deleteLater();
    }

    for(const auto &it : m_pEngine->factories()) {
        QUrl url(it.second.c_str());

        QObject *item       = m_rootItem;
        QStringList list    = url.path().split("/", QString::SkipEmptyParts);
        int i   = 0;
        for(const auto& part : list) {
            QObject *p  = item;
            item        = 0;
            foreach(QObject *it, p->children()) {
                if(part == it->objectName()) {
                    item    = it;
                    break;
                }
            }
            if(!item) {
                item    = new QObject(p);
                item->setObjectName(part);
                item->setProperty(URI, url.host());
            }
            i++;
        }
    }

    emit layoutAboutToBeChanged();
    emit layoutChanged();
}
