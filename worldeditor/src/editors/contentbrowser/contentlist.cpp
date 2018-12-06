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

const QString gIcon("icon");

ContentList *ContentList::m_pInstance   = nullptr;


ContentList::ContentList() :
        BaseObjectModel(nullptr),
        m_pEngine(nullptr) {

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
    m_pEngine       = engine;

    connect(m_pAssetManager, SIGNAL(directoryChanged(QString)), this, SLOT(update(QString)));

    scan(m_pProjectManager->resourcePath());
    scan(m_pProjectManager->contentPath());

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
        } break;
        case Qt::DisplayRole: {
            switch(index.column()) {
                case 1:     return info.path();
                case 2:     return info.isFile();
                case 3:     return item->property(qPrintable(gType));
                default:    return info.baseName();
            }
        } break;
        case Qt::SizeHintRole: {
            return QSize(m_DefaultIcon.width() + 16, m_DefaultIcon.height() + 32);
        } break;
        case Qt::DecorationRole: {
            QImage img  = item->property(qPrintable(gIcon)).value<QImage>();
            if(!img.isNull()) {
                return (img.height() < img.width()) ? img.scaledToWidth(m_DefaultIcon.width()) : img.scaledToHeight(m_DefaultIcon.height());
            }
        }
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
        }
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

QModelIndex ContentList::findResource(const QString &resource) const {
    QObject *item   = m_rootItem->findChild<QObject *>(resource);
    if(item) {
        return createIndex(item->parent()->children().indexOf(item), 0, item);
    }
    return QModelIndex();
}

QImage ContentList::icon(const QModelIndex &index) const {
    QObject *item   = static_cast<QObject *>(index.internalPointer());
    if(item) {
        return item->property(qPrintable(gIcon)).value<QImage>();
    }
    return QImage();
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

bool ContentList::canDropMimeData(const QMimeData *data, Qt::DropAction, int, int, const QModelIndex &parent) const {
    QFileInfo target;
    if(!parent.isValid()) {
        target  = QFileInfo(m_rootPath);
    } else {
        QObject *item   = static_cast<QObject *>(parent.internalPointer());
        target  = QFileInfo (m_pProjectManager->contentPath() + QDir::separator() + item->objectName());
    }
    bool result = target.isDir();
    if(result) {
        if(data->hasFormat(gMimeContent)) {
            QStringList list    = QString(data->data(gMimeContent)).split(";");
            foreach(QString path, list) {
                if( !path.isEmpty() ) {
                    QFileInfo source(m_pProjectManager->contentPath() + QDir::separator() + path);
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

bool ContentList::dropMimeData(const QMimeData *data, Qt::DropAction, int, int, const QModelIndex &parent) {
    QFileInfo target;
    if(!parent.isValid()) {
        target  = QFileInfo(m_rootPath);
    } else {
        QObject *item   = static_cast<QObject *>(parent.internalPointer());
        target  = QFileInfo (item->objectName());
    }
    if(data->hasUrls()) {
        foreach (const QUrl &url, data->urls()) {
            m_pAssetManager->import(QFileInfo(url.toLocalFile()), target);
        }
    } else if(data->hasFormat(gMimeContent)) {
        QStringList list    = QString(data->data(gMimeContent)).split(";");
        foreach(QString path, list) {
            if( !path.isEmpty() ) {
                QFileInfo info(path);
                m_pAssetManager->renameResource(info.filePath(), target.filePath() + "/" + info.fileName());
            }
        }
    } else if(data->hasFormat(gMimeObject)) {
         QStringList list   = QString(data->data(gMimeObject)).split(";");
         foreach(QString path, list) {
             if( !path.isEmpty() ) {
                 m_pAssetManager->makePrefab(path, target.filePath());
             }
         }
    }
    return true;
}

void ContentList::update(const QString &path) {
    QDir dir(m_pProjectManager->contentPath());
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
    QDir dir(m_pProjectManager->contentPath());

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
            item->setProperty(qPrintable(gIcon), m_DefaultIcon);
            if(!info.isDir()) {
                item->setProperty(qPrintable(gType), m_pAssetManager->resourceType(source));
                item->setProperty(qPrintable(gIcon), m_pAssetManager->icon(source) );
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
