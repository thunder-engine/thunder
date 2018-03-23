#include "contenttree.h"

#include <QDirIterator>
#include <QFileSystemWatcher>
#include <QUrl>

#include <projectmanager.h>

#include "config.h"
#include "contentlist.h"
#include "assetmanager.h"

ContentTree::ContentTree(QObject *parent) :
        BaseObjectModel(parent) {

    m_pContent  = new QObject(m_rootItem);

    QDir dir(ProjectManager::instance()->contentPath());
    m_pContent->setObjectName(dir.absolutePath());

    m_Folder    = QImage(":/Images/Folder.png").scaled(16, 16);

    connect(AssetManager::instance(), SIGNAL(directoryChanged(QString)), this, SLOT(update(QString)));

    update(dir.absolutePath());
}

int ContentTree::columnCount(const QModelIndex &parent) const {
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
            return info.baseName();
        }
        case Qt::DecorationRole: {
            return m_Folder;
        }
        default: break;
    }

    return QVariant();
}

Qt::ItemFlags ContentTree::flags(const QModelIndex &index) const {
    return BaseObjectModel::flags(index) | Qt::ItemIsSelectable;
}

QString ContentTree::dirPath(const QModelIndex &index) const {
    if(!index.isValid()) {
        return m_pContent->objectName();
    }

    QObject *item   = static_cast<QObject *>(index.internalPointer());
    return item->objectName();
}

void ContentTree::update(const QString &path) {
    QDir content(path);
    QObject *parent = m_rootItem->findChild<QObject *>(content.absolutePath());
    if(parent) {
        foreach(QObject *it, parent->children()) {
            QDir dir(it->objectName());
            if(!dir.exists()) {
                it->setParent(NULL);
                delete it;
            }
        }
    }

    QDirIterator it(path, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while(it.hasNext()) {
        QDir dir(it.next());
        if(!m_rootItem->findChild<QObject *>(dir.absolutePath())) {
            QDir parentDir  = dir;
            parent      = m_rootItem->findChild<QObject *>(content.absolutePath());
            if(parentDir.cdUp() && parentDir != content) {
                parent  = m_rootItem->findChild<QObject *>(parentDir.absolutePath());
            }
            if(parent) {
                QObject *item   = new QObject(parent);
                item->setObjectName(dir.absolutePath());
            }
        }
    }

    emit layoutAboutToBeChanged();
    emit layoutChanged();
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

bool ContentTree::canDropMimeData(const QMimeData *data, Qt::DropAction, int, int, const QModelIndex &parent) const {
    QFileInfo target(ProjectManager::instance()->contentPath());
    if(parent.isValid()) {
        QObject *item   = static_cast<QObject *>(parent.internalPointer());
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

bool ContentTree::dropMimeData(const QMimeData *data, Qt::DropAction, int, int, const QModelIndex &parent) {
    QDir dir(ProjectManager::instance()->contentPath());
    QFileInfo target;
    if(parent.isValid()) {
        QObject *item   = static_cast<QObject *>(parent.internalPointer());
        if(item != m_pContent) {
            target  = QFileInfo(dir.relativeFilePath(item->objectName()));
        }
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

