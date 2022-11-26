#include "contenttree.h"

#include <QDirIterator>
#include <QFileSystemWatcher>
#include <QUrl>
#include <QMimeData>

#include <editor/projectmanager.h>

#include "config.h"
#include "assetmanager.h"

#include <editor/projectmanager.h>

ContentTree *ContentTree::m_pInstance = nullptr;

ContentTreeFilter::ContentTreeFilter(QObject *parent) :
        QSortFilterProxyModel(parent) {

    sort(0);
}

void ContentTreeFilter::setContentTypes(const QStringList &list) {
    m_List = list;
    invalidate();
}

bool ContentTreeFilter::canDropMimeData(const QMimeData *data, Qt::DropAction, int, int, const QModelIndex &parent) const {
    QModelIndex index = mapToSource(parent);

    QFileInfo target(ProjectManager::instance()->contentPath());
    if(index.isValid()) {
        QObject *item = static_cast<QObject *>(index.internalPointer());
        target = QFileInfo(ProjectManager::instance()->contentPath() + QDir::separator() + item->objectName());
    }

    bool result = target.isDir();
    if(result) {
        if(data->hasFormat(gMimeContent)) {
            QStringList list = QString(data->data(gMimeContent)).split(";");
            foreach(QString path, list) {
                if( !path.isEmpty() ) {
                    QFileInfo source(ProjectManager::instance()->contentPath() + QDir::separator() + path);
                    result &= (source.absolutePath() != target.absoluteFilePath());
                    result &= (source != target);
                }
            }
        }
        if(data->hasFormat(gMimeObject)) {
            result = true;
        }
    }
    return result;
}

bool ContentTreeFilter::dropMimeData(const QMimeData *data, Qt::DropAction, int, int, const QModelIndex &parent) {
    QModelIndex index = mapToSource(parent);

    QDir dir(ProjectManager::instance()->contentPath());
    QFileInfo target;
    if(index.isValid()) {
        QObject *item = static_cast<QObject *>(index.internalPointer());
        target = QFileInfo(dir.relativeFilePath(item->objectName()));
    }

    if(data->hasUrls()) {
        foreach(const QUrl &url, data->urls()) {
            AssetManager::instance()->import(QFileInfo(url.toLocalFile()), target);
        }
    } else if(data->hasFormat(gMimeContent)) {
        QStringList list = QString(data->data(gMimeContent)).split(";");
        foreach(QString path, list) {
            if( !path.isEmpty() ) {
                QFileInfo source = QFileInfo(path);
                AssetManager::instance()->renameResource(dir.relativeFilePath(source.filePath()),
                                                         ((!target.filePath().isEmpty()) ? (target.filePath() + "/") :
                                                                                           QString("")) + source.fileName());
            }
        }
    } else if(data->hasFormat(gMimeObject)) {
        QStringList list = QString(data->data(gMimeObject)).split(";");
        foreach(QString path, list) {
            if( !path.isEmpty() ) {
                AssetManager::instance()->makePrefab(path, target.filePath());
            }
        }
    }
    return true;
}

bool ContentTreeFilter::lessThan(const QModelIndex &left, const QModelIndex &right) const {
    bool asc = sortOrder() == Qt::AscendingOrder ? true : false;

    bool isFile1 = sourceModel()->data(sourceModel()->index(left.row(), 1, left.parent()), Qt::DisplayRole).toBool();
    bool isFile2 = sourceModel()->data(sourceModel()->index(right.row(), 1, right.parent()), Qt::DisplayRole).toBool();
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
    QModelIndex index = sourceModel()->index(sourceRow, 2, sourceParent);
    for(auto &it : m_List) {
        QString type = sourceModel()->data(index).toString();
        if(it == type || type.isEmpty()) {
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

    m_content = new QObject(m_rootItem);
    m_content->setObjectName("Content");

    m_folder = QImage(":/Style/styles/dark/images/folder.svg");

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

bool ContentTree::isDir(const QModelIndex &index) const {
    QObject *item = static_cast<QObject *>(index.internalPointer());
    return QFileInfo(ProjectManager::instance()->contentPath() + QDir::separator() + item->objectName()).isDir();
}

int ContentTree::columnCount(const QModelIndex &) const {
    return 1;
}

QVariant ContentTree::data(const QModelIndex &index, int role) const {
    if(!index.isValid()) {
        return QVariant();
    }

    QObject *item = static_cast<QObject *>(index.internalPointer());
    QFileInfo info(ProjectManager::instance()->contentPath() + "/" + item->objectName());
    switch(role) {
        case Qt::EditRole:
        case Qt::DisplayRole: {
            switch(index.column()) {
                case 1:  return info.isFile();
                case 2:  return item->property(qPrintable(gType));
                default: return info.baseName();
            }
        }
        case Qt::DecorationRole: {
            return item->property(qPrintable(gIcon)).value<QImage>();
        }
        default: break;
    }

    return QVariant();
}

bool ContentTree::setData(const QModelIndex& index, const QVariant& value, int role) {
    Q_UNUSED(role)
    switch(index.column()) {
        case 0: {
            QDir dir(ProjectManager::instance()->contentPath());
            QObject *item = static_cast<QObject *>(index.internalPointer());
            QFileInfo info(item->objectName());
            QString path = (info.path() != ".") ? (info.path() + QDir::separator()) : "";
            QString suff = (!info.suffix().isEmpty()) ? ("." + info.suffix()) : "";
            QFileInfo dest(path + value.toString() + suff);
            AssetManager::instance()->renameResource(dir.relativeFilePath(info.filePath()),
                                            dir.relativeFilePath(dest.filePath()));
        } break;
        default: break;
    }
    return true;
}

Qt::ItemFlags ContentTree::flags(const QModelIndex &index) const {
    Qt::ItemFlags flags = BaseObjectModel::flags(index) | Qt::ItemIsSelectable;
    if(index.parent().isValid()) {
        flags |= Qt::ItemIsEditable;
    }
    return flags;
}

QString ContentTree::path(const QModelIndex &index) const {
    if(index.isValid()) {
        QObject *item = static_cast<QObject *>(index.internalPointer());
        if(item && item != m_content) {
            return item->objectName();
        }
    }

    return QString();
}

void ContentTree::onRendered(const QString &uuid) {
    AssetManager *mgr = AssetManager::instance();
    QString path = ProjectManager::instance()->contentPath() + "/" + mgr->guidToPath(uuid.toStdString()).c_str();
    QObject *item = m_rootItem->findChild<QObject *>(path);
    if(item) {
        item->setProperty(qPrintable(gType), mgr->assetTypeName(path));

        QImage img = mgr->icon(path);
        if(!img.isNull()) {
            item->setProperty(qPrintable(gIcon), (img.height() < img.width()) ? img.scaledToWidth(m_folder.width()) :
                                                                                img.scaledToHeight(m_folder.height()));
        }

        emit layoutAboutToBeChanged();
        emit layoutChanged();
    }
}

bool ContentTree::reimportResource(const QModelIndex &index) {
    QObject *item = static_cast<QObject *>(index.internalPointer());
    AssetManager::instance()->pushToImport(ProjectManager::instance()->contentPath() + "/" + item->objectName());
    AssetManager::instance()->reimport();
    return true;
}

bool ContentTree::removeResource(const QModelIndex &index) {
    if(index.isValid()) {
        QObject *item = static_cast<QObject *>(index.internalPointer());
        if(item) {
            AssetManager::instance()->removeResource(QFileInfo(item->objectName()));
            item->setParent(nullptr);
            delete item;
        }
    }

    emit layoutAboutToBeChanged();
    emit layoutChanged();
    return true;
}

QModelIndex ContentTree::getContent() const {
    return getIndex(m_content, QModelIndex());
}

void ContentTree::update(const QString &path) {
    QDir dir(ProjectManager::instance()->contentPath());

    QObject *parent = m_rootItem->findChild<QObject *>(dir.relativeFilePath(path));
    if(parent == nullptr) {
        parent = m_content;
    }
    clean(parent);

    AssetManager *instance = AssetManager::instance();

    QDirIterator it(path, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while(it.hasNext()) {
        QFileInfo info(it.next());
        if((QString(".") + info.suffix()) == gMetaExt) {
            continue;
        }

        parent = m_rootItem->findChild<QObject *>(dir.relativeFilePath(info.absolutePath()));
        if(parent == nullptr) {
            parent = m_content;
        }
        QString source = info.absoluteFilePath().contains(dir.absolutePath()) ?
                             dir.relativeFilePath(info.absoluteFilePath()) :
                             (QString(".embedded/") + info.fileName());
        if(parent->findChild<QObject *>(source) == nullptr) {
            QObject *item = new QObject(parent);
            item->setObjectName(source);
            if(!info.isDir()) {
                item->setProperty(qPrintable(gType), instance->assetTypeName(info));
                item->setProperty(qPrintable(gIcon), instance->icon(source));
            } else {
                item->setProperty(qPrintable(gIcon), m_folder);
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
            it->deleteLater();
        } else {
            clean(it);
        }
    }
}

Qt::DropActions ContentTree::supportedDropActions() const {
    return Qt::CopyAction | Qt::MoveAction;
}

QStringList ContentTree::mimeTypes() const {
    return {gMimeContent};
}

QMimeData *ContentTree::mimeData(const QModelIndexList &indexes) const {
    QStringList list;
    foreach(QModelIndex index, indexes) {
        if (index.isValid()) {
            QObject *item = static_cast<QObject *>(index.internalPointer());
            QString path = item->objectName();
            if(!path.isEmpty()) {
                list << path;
            }
        }
    }
    if(list.isEmpty()) {
        return nullptr;
    }
    QMimeData *mimeData = new QMimeData();
    mimeData->setData(gMimeContent, qPrintable(list.join(";")));
    return mimeData;
}
