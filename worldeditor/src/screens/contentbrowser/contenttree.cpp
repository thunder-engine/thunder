#include "contenttree.h"

#include "config.h"

#include <QDir>
#include <QMimeData>

#include <url.h>
#include <file.h>

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
    TString path(ProjectSettings::instance()->contentPath() + "/" + item->objectName().toStdString());
    switch(role) {
        case Qt::EditRole:
        case Qt::DisplayRole: {
            switch(index.column()) {
                case 1:  return File::isDir(path);
                case 2:  return item->property(gType);
                default: return Url(path).baseName().data();
            }
        }
        case Qt::DecorationRole: {
            return item->property(gIcon).value<QImage>();
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

            Url url(item->objectName().toStdString());
            if(item == m_newAsset) {
                TString source(m_newAsset->property(gImport).toString().toStdString());
                TString path = ProjectSettings::instance()->contentPath() + "/" + url.dir();
                if(source.isEmpty()) {
                    QDir dir(path.data());
                    dir.mkdir(value.toString());
                } else {
                    AssetManager::instance()->createFromTemplate(path + "/" + value.toString().toStdString() + "." + Url(source).suffix());
                }

                m_newAsset->setParent(nullptr);
            } else {
                TString path = ProjectSettings::instance()->contentPath() + "/" + url.dir();
                if(path != ".") {
                    path += "/";
                }
                TString dest(path + value.toString().toStdString());
                if(!url.suffix().isEmpty()) {
                    dest += TString(".") + url.suffix();
                };

                AssetManager::instance()->renameResource(ProjectSettings::instance()->contentPath() + "/" + url.path(), dest);
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

    AssetConverterSettings *settings = asset->fetchSettings(asset->uuidToPath(uuid));
    if(settings) {
        settings->resetIcon(uuid);
    }

    TString path(asset->uuidToPath(uuid));
    QString source = path.contains(dir.absolutePath().toStdString()) ?
                         dir.relativeFilePath(path.data()) :
                         (QString(".embedded/") + Url(path).name().data());

    QObject *item(m_rootItem->findChild<QObject *>(source));
    if(item) {
        item->setProperty(gType, asset->assetTypeName(path).data());

        QImage img = asset->icon(path);
        if(!img.isNull()) {
            item->setProperty(gIcon, (img.height() < img.width()) ? img.scaledToWidth(m_folder.width()) :
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
        Url url(name.toStdString());

        QDir dir(ProjectSettings::instance()->contentPath().data());

        TString path = dir.relativeFilePath(url.absoluteDir().data()).toStdString();
        QObject *parent = m_rootItem->findChild<QObject *>(path.data());
        if(parent == nullptr) {
            parent = m_content;
        }

        m_newAsset->setParent(parent);
        m_newAsset->setObjectName(dir.relativeFilePath(name));
        m_newAsset->setProperty(gImport, source);

        AssetConverterSettings *settings = AssetManager::instance()->fetchSettings(source.toStdString());
        if(settings) {
            m_newAsset->setProperty(gIcon, (directory) ? m_folder : settings->icon(settings->destination()));
        }
    } else {
        m_newAsset->setParent(nullptr);
    }

    emit layoutAboutToBeChanged();
    emit layoutChanged();

    return getIndex(m_newAsset);
}

void ContentTree::update() {
    QString path(ProjectSettings::instance()->contentPath().data());
    QDir dir(path);

    QObject *parent = m_rootItem->findChild<QObject *>(dir.relativeFilePath(path));
    if(parent == nullptr) {
        parent = m_content;
    }
    clean(parent);

    AssetManager *asset = AssetManager::instance();

    StringList list = File::list(path.toStdString());
    for(TString &path : list) {
        Url info(path);
        if(info.suffix() == gMetaExt) {
            continue;
        }

        parent = m_rootItem->findChild<QObject *>(dir.relativeFilePath(info.absoluteDir().data()));
        if(parent == nullptr) {
            parent = m_content;
        }
        QString source = path.contains(dir.absolutePath().toStdString()) ?
                             dir.relativeFilePath(path.data()) :
                             (QString(".embedded/") + info.name().data());
        if(parent->findChild<QObject *>(source) == nullptr) {
            QObject *item = new QObject(parent);
            item->setObjectName(source);
            if(!File::isDir(path)) {
                item->setProperty(gType, asset->assetTypeName(path).data());
                item->setProperty(gIcon, asset->icon(path));
            } else {
                item->setProperty(gIcon, m_folder);
            }
        }
    }

    emit layoutAboutToBeChanged();
    emit layoutChanged();
}

void ContentTree::clean(QObject *parent) {
    foreach(QObject *it, parent->children()) {
        if(!File::exists(it->objectName().toStdString())) {
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
