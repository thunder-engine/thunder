#include "functionmodel.h"

#include <QMetaClassInfo>
#include <QUrl>

#define URI "uri"

FunctionModel::FunctionModel(const QStringList &classes) :
        BaseObjectModel(nullptr) {
    m_Classes   = classes;
    update();

}

int FunctionModel::columnCount(const QModelIndex &parent) const {
    return 3;
}

QVariant FunctionModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const {
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0: return tr("Name");
        }
    }
    return QVariant();
}

QVariant FunctionModel::data(const QModelIndex &index, int role) const {
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

void FunctionModel::update() {
    int start   = rowCount();

    foreach(QString it, m_Classes) {
        const QByteArray className = qPrintable(it);
        const int type = QMetaType::type( className );
        const QMetaObject *meta = QMetaType::metaObjectForType(type);
        if(meta) {
            int index   = meta->indexOfClassInfo("Group");
            if(index != -1) {
                QUrl url(QString(meta->classInfo(index).value()) + "/" + it);

                QObject *pItem      = m_rootItem;
                QStringList list    = url.path().split("/", QString::SkipEmptyParts);
                int i   = 0;
                for(const auto& part : list) {
                    QObject *p  = pItem;
                    pItem       = 0;
                    foreach(QObject *item, p->children()) {
                        if(part == item->objectName()) {
                            pItem = item;
                            break;
                        }
                    }
                    if(!pItem) {
                        pItem   = new QObject(p);
                        pItem->setObjectName(part);
                        pItem->setProperty(URI, url.host());
                    }
                    i++;
                }
            }
        }
    }

    int count   = rowCount() - 1;
    if(count > 0) {
        beginInsertRows(QModelIndex(), start, start + count);

        endInsertRows();
    }
}
