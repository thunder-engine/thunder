#include "contentlist.h"

#include <QDirIterator>
#include <QDebug>
#include <QUrl>

#include <file.h>
#include <log.h>
#include <engine.h>

#include "config.h"

#include "assetmanager.h"
#include "projectmanager.h"

ContentList *ContentList::m_pInstance   = nullptr;

ContentListFilter::ContentListFilter(QObject *parent) :
        QSortFilterProxyModel(parent) {

    sort(0);
}

void ContentListFilter::setContentTypes(const TypeList &list) {
    m_List = list;
    invalidate();
}

void ContentListFilter::setRootPath(const QString &path) {
    m_rootPath = path;
    invalidate();
}

QString ContentListFilter::rootPath() const {
    return m_rootPath;
}

bool ContentListFilter::canDropMimeData(const QMimeData *data, Qt::DropAction, int, int, const QModelIndex &parent) const {
    ProjectManager *mgr = ProjectManager::instance();

    QModelIndex index   = mapToSource(parent);

    QFileInfo target;
    if(!index.isValid()) {
        target  = QFileInfo(m_rootPath);
    } else {
        QObject *item   = static_cast<QObject *>(index.internalPointer());
        target  = QFileInfo (mgr->contentPath() + QDir::separator() + item->objectName());
    }
    bool result = target.isDir();
    if(result) {
        if(data->hasFormat(gMimeContent)) {
            QStringList list = QString(data->data(gMimeContent)).split(";");
            foreach(QString path, list) {
                if( !path.isEmpty() ) {
                    QFileInfo source(mgr->contentPath() + QDir::separator() + path);
                    result  &= (source.absolutePath() != target.absoluteFilePath());
                    result  &= (source != target);
                }
            }
        }
        if(data->hasFormat(gMimeObject)) {
            result   = true;
        }
    }
    return result;
}

bool ContentListFilter::dropMimeData(const QMimeData *data, Qt::DropAction, int, int, const QModelIndex &parent) {
    QModelIndex index   = mapToSource(parent);

    QFileInfo target;
    if(!index.isValid()) {
        target  = QFileInfo(m_rootPath);
    } else {
        QObject *item   = static_cast<QObject *>(index.internalPointer());
        target  = QFileInfo (item->objectName());
    }

    AssetManager *mgr = AssetManager::instance();

    if(data->hasUrls()) {
        foreach (const QUrl &url, data->urls()) {
            mgr->import(QFileInfo(url.toLocalFile()), target);
        }
    } else if(data->hasFormat(gMimeContent)) {
        QStringList list    = QString(data->data(gMimeContent)).split(";");
        foreach(QString path, list) {
            if( !path.isEmpty() ) {
                QFileInfo info(path);
                mgr->renameResource(info.filePath(), target.filePath() + "/" + info.fileName());
            }
        }
    } else if(data->hasFormat(gMimeObject)) {
         QStringList list   = QString(data->data(gMimeObject)).split(";");
         foreach(QString path, list) {
             if( !path.isEmpty() ) {
                 mgr->makePrefab(path, target.filePath());
             }
         }
    }
    return true;
}

bool ContentListFilter::lessThan(const QModelIndex &left, const QModelIndex &right) const {
    bool asc = sortOrder() == Qt::AscendingOrder ? true : false;

    ContentList *inst   = ContentList::instance();
    bool isFile1    = inst->data(inst->index(left.row(), 2, left.parent()), Qt::DisplayRole).toBool();
    bool isFile2    = inst->data(inst->index(right.row(), 2, right.parent()), Qt::DisplayRole).toBool();
    if(!isFile1 && isFile2) {
        return asc;
    }
    if(isFile1 && !isFile2) {
        return !asc;
    }
    return QSortFilterProxyModel::lessThan(left, right);
}

bool ContentListFilter::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
    bool result = checkRootPath(sourceRow, sourceParent);
    if(!m_List.isEmpty()) {
        result = checkContentTypeFilter(sourceRow, sourceParent);
    }
    result &= checkNameFilter(sourceRow, sourceParent);
    return result;
}

bool ContentListFilter::checkContentTypeFilter(int sourceRow, const QModelIndex &sourceParent) const {
    QModelIndex index   = sourceModel()->index(sourceRow, 3, sourceParent);
    foreach (int32_t it, m_List) {
        int32_t type    = sourceModel()->data(index).toInt();
        if(it == type) {
            return true;
        }
    }
    return false;
}

bool ContentListFilter::checkRootPath(int sourceRow, const QModelIndex &sourceParent) const {
    QString path = sourceModel()->data(sourceModel()->index(sourceRow, 1, sourceParent)).toString();
    return (rootPath() == path);
}

bool ContentListFilter::checkNameFilter(int sourceRow, const QModelIndex &sourceParent) const {
    QModelIndex index = sourceModel()->index(sourceRow, 2, sourceParent);
    return (QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent) && (filterRegExp().isEmpty() || sourceModel()->data(index).toBool()));
}

ContentList::ContentList() :
        BaseObjectModel(nullptr),
        m_pEngine(nullptr),
        m_pContent(nullptr) {

    m_pProjectManager   = ProjectManager::instance();
    m_pAssetManager     = AssetManager::instance();

    m_DefaultIcon   = QImage(":/Images/Folder.png");
}

ContentList *ContentList::instance() {
    if(!m_pInstance) {
        m_pInstance = new ContentList;
    }
    return m_pInstance;
}

void ContentList::destroy() {
    delete m_pInstance;
    m_pInstance = nullptr;
}

void ContentList::init(Engine *engine) {
    m_pEngine = engine;

    connect(m_pAssetManager, SIGNAL(directoryChanged(QString)), this, SLOT(update()));

    emit layoutAboutToBeChanged();
    emit layoutChanged();
}

int ContentList::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return 4;
}

QVariant ContentList::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const {
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 1:     return tr("Path");
            case 2:     return tr("isFile");
            case 3:     return gType;
            default:    return tr("Name");
        }
    }
    return QVariant();
}

QVariant ContentList::data(const QModelIndex &index, int role) const {
    if(!index.isValid()) {
        return QVariant();
    }

    QObject *item   = static_cast<QObject *>(index.internalPointer());
    QFileInfo info(m_pProjectManager->contentPath() + "/" + item->objectName());
    switch(role) {
        case Qt::EditRole: {
            return info.baseName();
        }
        case Qt::DisplayRole: {
            switch(index.column()) {
                case 1:     return info.path();
                case 2:     return info.isFile();
                case 3:     return item->property(qPrintable(gType));
                default:    return info.baseName();
            }
        }
        case Qt::SizeHintRole: {
            return QSize(m_DefaultIcon.width() + 16, m_DefaultIcon.height() + 32);
        }
        case Qt::DecorationRole: {
            QImage img  = item->property(qPrintable(gIcon)).value<QImage>();
            if(!img.isNull()) {
                return (img.height() < img.width()) ? img.scaledToWidth(m_DefaultIcon.width()) : img.scaledToHeight(m_DefaultIcon.height());
            }
        } break;
        case Qt::ToolTipRole: {
            return info.path();
        }
        default: break;
    }
    return QVariant();
}

Qt::ItemFlags ContentList::flags(const QModelIndex &index) const {
    Qt::ItemFlags result    = BaseObjectModel::flags(index);
    result |= Qt::ItemIsEditable | Qt::ItemIsSelectable;
    return result;
}

bool ContentList::setData(const QModelIndex &index, const QVariant &value, int role) {
    Q_UNUSED(role)
    switch(index.column()) {
        case 0: {
            QDir dir(m_pProjectManager->contentPath());
            QObject *item   = static_cast<QObject *>(index.internalPointer());
            QFileInfo info(item->objectName());
            QString path    = (info.path() != ".") ? (info.path() + QDir::separator()) : "";
            QString suff    = (!info.suffix().isEmpty()) ? ("." + info.suffix()) : "";
            QFileInfo dest(path + value.toString() + suff);
            m_pAssetManager->renameResource(dir.relativeFilePath(info.filePath()),
                                            dir.relativeFilePath(dest.filePath()));
        } break;
        default: break;
    }
    return true;
}

bool ContentList::isDir(const QModelIndex &index) const {
    QObject *item   = static_cast<QObject *>(index.internalPointer());
    return QFileInfo(m_pProjectManager->contentPath() + QDir::separator() + item->objectName()).isDir();
}

bool ContentList::reimportResource(const QModelIndex &index) {
    QObject *item   = static_cast<QObject *>(index.internalPointer());
    m_pAssetManager->pushToImport(m_pProjectManager->contentPath() + QDir::separator() + item->objectName());
    m_pAssetManager->reimport();
    return true;
}

bool ContentList::removeResource(const QModelIndex &index) {
    if(index.isValid()) {
        QObject *item   = static_cast<QObject *>(index.internalPointer());
        if(item) {
            m_pAssetManager->removeResource(QFileInfo(item->objectName()));

            item->setParent(nullptr);
            delete item;
        }
    }
    return true;
}

QString ContentList::path(const QModelIndex &index) const {
    QObject *item   = static_cast<QObject *>(index.internalPointer());
    if(item) {
        return item->objectName();
    }
    return QString();
}

Qt::DropActions ContentList::supportedDropActions() const {
    return Qt::CopyAction | Qt::MoveAction;
}

QStringList ContentList::mimeTypes() const {
    QStringList types;
    types << gMimeContent;
    return types;
}

QMimeData *ContentList::mimeData(const QModelIndexList &indexes) const {
    QMimeData *mimeData = new QMimeData();

    QStringList list;
    foreach (QModelIndex index, indexes) {
        if (index.isValid()) {
            QObject *item   = static_cast<QObject *>(index.internalPointer());
            QString path(item->objectName());
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

void ContentList::update() {
    QString path = m_pProjectManager->contentPath();
    QDir dir(path);

    QObject *parent = m_rootItem->findChild<QObject *>(dir.relativeFilePath(path));
    if(!parent) {
        parent  = m_rootItem;
    }

    foreach(QObject *it, m_rootItem->children()) {
        QFileInfo info(dir.absoluteFilePath(it->objectName()));
        if(!info.exists()) {
            it->setParent(nullptr);
            delete it;
        }
    }
    scan(path);

    emit layoutAboutToBeChanged();
    emit layoutChanged();
}

void ContentList::scan(const QString &path) {
    QDir dir(path);

    QDirIterator it(path, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while(it.hasNext()) {
        QFileInfo info(it.next());
        if((QString(".") + info.suffix()) == gMetaExt) {
            continue;
        }

        QString source = info.absoluteFilePath().contains(dir.absolutePath()) ?
                             dir.relativeFilePath(info.absoluteFilePath()) : (QString(".embedded/") + info.fileName());

        if(!m_rootItem->findChild<QObject *>(source)) {
            QObject *item   = new QObject(m_rootItem);
            item->setObjectName(source);
            if(!info.isDir()) {
                item->setProperty(qPrintable(gType), m_pAssetManager->resourceType(source));
                item->setProperty(qPrintable(gIcon), m_pAssetManager->icon(source) );
            } else {
                item->setProperty(qPrintable(gIcon), m_DefaultIcon);
            }
        }
    }
}

void ContentList::onRendered(const QString &uuid) {
    QString path    = m_pAssetManager->guidToPath(uuid.toStdString()).c_str();
    QObject *item   = m_rootItem->findChild<QObject *>(path);
    if(item) {
        item->setProperty(qPrintable(gType), m_pAssetManager->resourceType(path));
        item->setProperty(qPrintable(gIcon), m_pAssetManager->icon(path));
    }
    emit layoutAboutToBeChanged();
    emit layoutChanged();
}
