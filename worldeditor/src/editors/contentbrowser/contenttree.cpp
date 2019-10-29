#include "contenttree.h"

#include <QDirIterator>
#include <QFileSystemWatcher>
#include <QUrl>

#include <projectmanager.h>

#include "config.h"
#include "contentlist.h"
#include "assetmanager.h"

ContentTree *ContentTree::m_pInstance   = nullptr;

ContentTreeFilter::ContentTreeFilter(QObject *parent) :
        QSortFilterProxyModel(parent) {

    sort(0);
}

void ContentTreeFilter::setContentTypes(const TypeList &list) {
    m_List = list;
    invalidate();
}

bool ContentTreeFilter::canDropMimeData(const QMimeData *data, Qt::DropAction, int, int, const QModelIndex &parent) const {
    QModelIndex index   = mapToSource(parent);

    QFileInfo target(ProjectManager::instance()->contentPath());
    if(index.isValid()) {
        QObject *item   = static_cast<QObject *>(index.internalPointer());
        target  = QFileInfo(item->objectName());
    }

    bool result = target.isDir();
    if(result && data->hasFormat(gMimeContent)) {
        QStringList list    = QString(data->data(gMimeContent)).split(";");
        foreach(QString path, list) {
            if( !path.isEmpty() ) {
                QFileInfo source(ProjectManager::instance()->contentPath() + QDir::separator() + path);
                result  &= (source.absolutePath() != target.absoluteFilePath());
                result  &= (source != target);
            }
        }
    }
    return result;
}

bool ContentTreeFilter::dropMimeData(const QMimeData *data, Qt::DropAction, int, int, const QModelIndex &parent) {
    QModelIndex index   = mapToSource(parent);

    QDir dir(ProjectManager::instance()->contentPath());
    QFileInfo target;
    if(index.isValid()) {
        QObject *item   = static_cast<QObject *>(index.internalPointer());
        //if(item != m_pContent) {
            target  = QFileInfo(dir.relativeFilePath(item->objectName()));
        //}
    }

    if(data->hasUrls()) {
        foreach (const QUrl &url, data->urls()) {
            AssetManager::instance()->import(QFileInfo(url.toLocalFile()), target);
        }
    } else if(data->hasFormat(gMimeContent)) {
        QStringList list    = QString(data->data(gMimeContent)).split(";");
        foreach(QString path, list) {
            if( !path.isEmpty() ) {
                QFileInfo source    = QFileInfo(path);
                AssetManager::instance()->renameResource(dir.relativeFilePath(source.filePath()),
                                                         ((!target.filePath().isEmpty()) ? (target.filePath() + "/") : QString("")) + source.fileName());
            }
        }
    }
    return true;
}

bool ContentTreeFilter::lessThan(const QModelIndex &left, const QModelIndex &right) const {
    bool asc = sortOrder() == Qt::AscendingOrder ? true : false;

    ContentTree *inst = ContentTree::instance();
    bool isFile1 = inst->data(inst->index(left.row(), 1, left.parent()), Qt::DisplayRole).toBool();
    bool isFile2 = inst->data(inst->index(right.row(), 1, right.parent()), Qt::DisplayRole).toBool();
    if(!isFile1 && isFile2) {
        return asc;
    }
    if(isFile1 && !isFile2) {
        return !asc;
    }
    return QSortFilterProxyModel::lessThan(left, right);
}

bool ContentTreeFilter::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
    bool result = true;
    if(!m_List.isEmpty()) {
        result = checkContentTypeFilter(sourceRow, sourceParent);
    }
    result &= checkNameFilter(sourceRow, sourceParent);
    return result;
}

bool ContentTreeFilter::checkContentTypeFilter(int sourceRow, const QModelIndex &sourceParent) const {
    QModelIndex index   = sourceModel()->index(sourceRow, 2, sourceParent);
    foreach (int32_t it, m_List) {
        int32_t type    = sourceModel()->data(index).toInt();
        if(it == type || type == 0) {
            return true;
        }
    }
    return false;
}

bool ContentTreeFilter::checkNameFilter(int sourceRow, const QModelIndex &sourceParent) const {
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    return (QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent) && (filterRegExp().isEmpty() || sourceModel()->data(index).toBool()));
}


ContentTree::ContentTree() :
        BaseObjectModel(nullptr) {

    m_pContent = new QObject(m_rootItem);
    m_Folder = QImage(":/Images/Folder.png").scaled(20, 20);

    connect(AssetManager::instance(), SIGNAL(directoryChanged(QString)), this, SLOT(update(QString)));
}

ContentTree *ContentTree::instance() {
    if(!m_pInstance) {
        m_pInstance = new ContentTree;
    }
    return m_pInstance;
}

void ContentTree::destroy() {
    delete m_pInstance;
    m_pInstance = nullptr;
}

int ContentTree::columnCount(const QModelIndex &) const {
    return 1;
}

QVariant ContentTree::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const {
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0: return tr("Name");
        }
    }
    return QVariant();
}

QVariant ContentTree::data(const QModelIndex &index, int role) const {
    if(!index.isValid()) {
        return QVariant();
    }

    QObject *item   = static_cast<QObject *>(index.internalPointer());
    switch(role) {
        case Qt::DisplayRole: {
            QFileInfo info(item->objectName());
            switch(index.column()) {
                case 1:  return info.isFile();
                case 2:  return item->property(qPrintable(gType));
                default: return info.fileName();
            }
        }
        case Qt::DecorationRole: {
            return item->property(qPrintable(gIcon)).value<QImage>();
        }
        default: break;
    }

    return QVariant();
}

bool ContentTree::setData(const QModelIndex& index, const QVariant& value, int role)
{
    Q_UNUSED(role)
    switch(index.column()) {
        case 0: {
            QDir dir(ProjectManager::instance()->contentPath());
            QObject *item   = static_cast<QObject *>(index.internalPointer());
            QFileInfo info(item->objectName());
            QString path    = (info.path() != ".") ? (info.path() + QDir::separator()) : "";
            QString suff    = (!info.suffix().isEmpty()) ? ("." + info.suffix()) : "";
            QFileInfo dest(path + value.toString() + suff);
            AssetManager::instance()->renameResource(dir.relativeFilePath(info.filePath()),
                                            dir.relativeFilePath(dest.filePath()));
        } break;
        default: break;
    }
    return true;
}

Qt::ItemFlags ContentTree::flags(const QModelIndex &index) const {
    return BaseObjectModel::flags(index) | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

QString ContentTree::path(const QModelIndex &index) const {
    if(!index.isValid()) {
        return m_pContent->objectName();
    }

    QObject *item   = static_cast<QObject *>(index.internalPointer());
    return item->objectName();
}

void ContentTree::onRendered(const QString &uuid) {
    AssetManager *mgr = AssetManager::instance();
    QString path    = ProjectManager::instance()->contentPath() + "/" + mgr->guidToPath(uuid.toStdString()).c_str();
    QObject *item   = m_rootItem->findChild<QObject *>(path);
    if(item) {
        item->setProperty(qPrintable(gType), mgr->resourceType(path));

        QImage img = mgr->icon(path);
        if(!img.isNull()) {
            item->setProperty(qPrintable(gIcon), (img.height() < img.width()) ? img.scaledToWidth(m_Folder.width()) : img.scaledToHeight(m_Folder.height()));
        }

        emit layoutAboutToBeChanged();
        emit layoutChanged();
    }
}

bool ContentTree::removeResource(const QModelIndex &index) {
    if(index.isValid()) {
        QObject *item   = static_cast<QObject *>(index.internalPointer());
        if(item) {
            QFileInfo fullName(item->objectName());
            AssetManager::instance()->removeResource(QFileInfo(fullName.fileName()));
            item->setParent(nullptr);
            delete item;
        }
    }

    emit layoutAboutToBeChanged();
    emit layoutChanged();
    return true;
}

void ContentTree::update(const QString &path) {
    QDir dir(ProjectManager::instance()->contentPath());
    m_pContent->setObjectName(dir.absolutePath());

    clean(m_rootItem);

    AssetManager *instance = AssetManager::instance();

    QDirIterator it(path, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while(it.hasNext()) {
        QFileInfo info(it.next());
        if((QString(".") + info.suffix()) == gMetaExt) {
            continue;
        }

        QObject *parent = m_rootItem->findChild<QObject *>(info.absolutePath());
        if(parent) {
            QString source = info.absoluteFilePath();
            if(parent->findChild<QObject *>(source) == nullptr) {
                QObject *item = new QObject(parent);
                item->setObjectName(source);
                if(!info.isDir()) {
                    int32_t type = instance->resourceType(source);
                    item->setProperty(qPrintable(gType), type);
                    QImage img = instance->icon(source);
                    if(!img.isNull()) {
                        item->setProperty(qPrintable(gIcon), (img.height() < img.width()) ? img.scaledToWidth(m_Folder.width()) : img.scaledToHeight(m_Folder.height()));
                    }
                } else {
                    item->setProperty(qPrintable(gIcon), m_Folder);
                }
            }
        }
    }

    emit layoutAboutToBeChanged();
    emit layoutChanged();
}

void ContentTree::clean(QObject *parent) {
    foreach(QObject *it, parent->children()) {
        QFileInfo dir(it->objectName());
        if(!dir.exists()) {
            it->setParent(nullptr);
            delete it;
        } else {
            clean(it);
        }
    }
}

Qt::DropActions ContentTree::supportedDropActions() const {
    return Qt::CopyAction | Qt::MoveAction;
}

QStringList ContentTree::mimeTypes() const {
    QStringList types;
    types << gMimeContent;
    return types;
}

QMimeData *ContentTree::mimeData(const QModelIndexList &indexes) const {
    QMimeData *mimeData = new QMimeData();

    QStringList list;
    foreach (QModelIndex index, indexes) {
        if (index.isValid()) {
            QObject *item   = static_cast<QObject *>(index.internalPointer());
            QString path    = item->objectName();
            if(!path.isEmpty()) {
                list << path;
            }
        }
    }
    if(list.isEmpty()) {
        return nullptr;
    }
    mimeData->setData(gMimeContent, qPrintable(list.join(";")));
    return mimeData;
}
