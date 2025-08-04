#include "contenttree.h"

#include "config.h"

#include <QDirIterator>
#include <QMimeData>

#include <editor/assetmanager.h>
#include <editor/projectsettings.h>

ContentTree::ContentTree() :
        BaseObjectModel(nullptr),
        m_content(new QObject(m_rootItem)),
        m_newAsset(new QObject) {

    m_content->setObjectName("Content");

    m_folder = QImage(":/Style/styles/dark/images/folder.svg");

    connect(AssetManager::instance(), &AssetManager::directoryChanged, this, &ContentTree::update);
    connect(AssetManager::instance(), &AssetManager::iconUpdated, this, &ContentTree::onRendered);
}

ContentTree *ContentTree::instance() {
    static ContentTree instance;
    return &instance;
}

int ContentTree::columnCount(const QModelIndex &) const {
    return 1;
}

QVariant ContentTree::data(const QModelIndex &index, int role) const {
    if(!index.isValid()) {
        return QVariant();
    }

    QObject *item = static_cast<QObject *>(index.internalPointer());
    QFileInfo info((ProjectSettings::instance()->contentPath() + "/" + item->objectName().toStdString()).data());
    switch(role) {
        case Qt::EditRole:
        case Qt::DisplayRole: {
            switch(index.column()) {
                case 1:  return info.isDir();
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

bool ContentTree::setData(const QModelIndex &index, const QVariant &value, int role) {
    Q_UNUSED(role)
    switch(index.column()) {
        case 0: {
            QObject *item = static_cast<QObject *>(index.internalPointer());
            QFileInfo info(item->objectName());
            if(item == m_newAsset) {
                QString source(m_newAsset->property(qPrintable(gImport)).toString());
                QString path = QString(ProjectSettings::instance()->contentPath().data()) + "/" + info.path();
                if(source.isEmpty()) {
                    QDir dir(path);
                    dir.mkdir(value.toString());
                } else {
                    AssetConverter *converter = AssetManager::instance()->getConverter(source.toStdString());
                    if(converter) {
                        converter->createFromTemplate((QString(path + "/" + value.toString() + "." + QFileInfo(source).suffix())).toStdString());
                    }
                }

                m_newAsset->setParent(nullptr);
            } else {
                QDir dir(ProjectSettings::instance()->contentPath().data());
                QString path = (info.path() != ".") ? (info.path() + "/") : "";
                QString suff = (!info.suffix().isEmpty()) ? ("." + info.suffix()) : "";
                QFileInfo dest(path + value.toString() + suff);
                AssetManager::instance()->renameResource(dir.relativeFilePath(info.filePath()).toStdString(),
                                                         dir.relativeFilePath(dest.filePath()).toStdString());
            }
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

void ContentTree::onRendered(const TString &uuid) {
    QDir dir(ProjectSettings::instance()->contentPath().data());

    AssetManager *asset = AssetManager::instance();

    AssetConverterSettings *settings = asset->fetchSettings(asset->guidToPath(uuid));
    if(settings) {
        settings->resetIcon(uuid);
    }

    QFileInfo info(asset->guidToPath(uuid).data());
    QString source = info.absoluteFilePath().contains(dir.absolutePath()) ?
                         dir.relativeFilePath(info.absoluteFilePath()) :
                         (QString(".embedded/") + info.fileName());

    QObject *item(m_rootItem->findChild<QObject *>(source));
    if(item) {
        item->setProperty(qPrintable(gType), asset->assetTypeName(info.absoluteFilePath().toStdString()).data());

        QImage img = asset->icon(info.absoluteFilePath().toStdString());
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
    AssetManager::instance()->pushToImport(ProjectSettings::instance()->contentPath() + "/" + item->objectName().toStdString());
    AssetManager::instance()->reimport();
    return true;
}

bool ContentTree::removeResource(const QModelIndex &index) {
    if(index.isValid()) {
        QObject *item = static_cast<QObject *>(index.internalPointer());
        if(item) {
            AssetManager::instance()->removeResource(item->objectName().toStdString());
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

QModelIndex ContentTree::setNewAsset(const QString &name, const QString &source, bool directory) {
    if(!name.isEmpty()) {
        QFileInfo info(name);

        QDir dir(ProjectSettings::instance()->contentPath().data());

        QString path = dir.relativeFilePath(info.path());
        QObject *parent = m_rootItem->findChild<QObject *>(path);
        if(parent == nullptr) {
            parent = m_content;
        }

        m_newAsset->setParent(parent);
        m_newAsset->setObjectName(dir.relativeFilePath(info.filePath()));
        m_newAsset->setProperty(qPrintable(gImport), source);

        AssetConverterSettings *settings = AssetManager::instance()->fetchSettings(source.toStdString());
        if(settings) {
            m_newAsset->setProperty(qPrintable(gIcon), (directory) ? m_folder : settings->icon(settings->destination()));
        }
    } else {
        m_newAsset->setParent(nullptr);
    }

    emit layoutAboutToBeChanged();
    emit layoutChanged();

    return getIndex(m_newAsset);
}

void ContentTree::update(const QString &path) {
    QDir dir(ProjectSettings::instance()->contentPath().data());

    QObject *parent = m_rootItem->findChild<QObject *>(dir.relativeFilePath(path));
    if(parent == nullptr) {
        parent = m_content;
    }
    clean(parent);

    AssetManager *asset = AssetManager::instance();

    QDirIterator it(path, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while(it.hasNext()) {
        QFileInfo info(it.next());
        if(info.suffix() == gMetaExt) {
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
                item->setProperty(qPrintable(gType), asset->assetTypeName(info.absoluteFilePath().toStdString()).data());
                item->setProperty(qPrintable(gIcon), asset->icon(info.absoluteFilePath().toStdString()));
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

void ContentTree::revert() {
    m_newAsset->setParent(nullptr);

    emit layoutAboutToBeChanged();
    emit layoutChanged();
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
        if(index.isValid()) {
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
