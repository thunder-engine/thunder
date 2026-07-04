#include "contenttree.h"

#include "config.h"

#include <QMimeData>

#include <url.h>
#include <file.h>

#include <editor/assetmanager.h>
#include <editor/assetconverter.h>
#include <editor/projectsettings.h>
#include <editor/codebuilder.h>

ContentTree::ContentTree() :
        BaseObjectModel(nullptr),
        m_content(new QObject(m_rootItem)),
        m_newAsset(new QObject) {

    addItem(m_content);
    addItem(m_newAsset);

    m_content->setObjectName("Content");

    m_folder = QImage(":/Style/styles/dark/images/folder.svg");

    connect(AssetManager::instance(), &AssetManager::directoryChanged, this, &ContentTree::update);
    connect(AssetManager::instance(), &AssetManager::iconUpdated, this, &ContentTree::onRendered);
}

int ContentTree::columnCount(const QModelIndex &) const {
    return 1;
}

QVariant ContentTree::data(const QModelIndex &index, int role) const {
    if(!index.isValid()) {
        return QVariant();
    }

    QObject *item = getObject(index);
    if(item) {
        TString path(item->objectName().toStdString());
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
    }

    return QVariant();
}

bool ContentTree::setData(const QModelIndex &index, const QVariant &value, int role) {
    Q_UNUSED(role)
    switch(index.column()) {
        case 0: {
            QObject *item = getObject(index);
            if(item) {
                Url url(item->objectName().toStdString());
                TString stringValue(value.toString().toStdString());
                TString dest(url.absoluteDir() + "/" + stringValue);
                if(!url.suffix().isEmpty()) {
                    dest += TString(".") + url.suffix();
                }
                if(item == m_newAsset) {
                    TString source(m_newAsset->property(gImport).toString().toStdString());
                    if(source.isEmpty()) {
                        File::mkDir(dest);
                    } else {
                        AssetManager *mgr = AssetManager::instance();
                        AssetConverter *converter = mgr->getConverter(dest);
                        if(converter) {
                            converter->createFromTemplate(dest);
                        } else {
                            TString suffix = url.suffix().toLower();
                            for(auto builder : mgr->builders()) {
                                for(auto &it : builder->suffixes()) {
                                    if(it == suffix) {
                                        builder->createFromTemplate(dest);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    m_newAsset->setParent(nullptr);
                } else {
                    AssetManager::instance()->renameResource(url.absoluteFilePath(), dest);
                }
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

TString ContentTree::path(const QModelIndex &index) const {
    if(index.isValid()) {
        QObject *item = getObject(index);
        if(item && item != m_content) {
            return item->objectName().toStdString();
        }
    }

    return TString(ProjectSettings::instance()->contentPath());
}

void ContentTree::onRendered(const TString &uuid) {
    AssetManager *asset = AssetManager::instance();

    TString path(ProjectSettings::instance()->contentPath() + "/" + asset->uuidToPath(uuid));
    AssetConverterSettings *settings = asset->fetchSettings(path);
    if(settings) {
        settings->resetIcon(uuid);

        QObject *item(m_rootItem->findChild<QObject *>(settings->source().data()));
        if(item) {
            item->setProperty(gType, asset->assetTypeName(path).data());

            QImage img = settings->icon(uuid);
            if(!img.isNull()) {
                item->setProperty(gIcon, img);
            }

            emit layoutAboutToBeChanged();
            emit layoutChanged();
        }
    }
}

bool ContentTree::reimportResource(const QModelIndex &index) {
    QObject *item = getObject(index);
    if(item) {
        AssetManager::instance()->pushToImport(item->objectName().toStdString());
        AssetManager::instance()->reimport();
    }
    return true;
}

bool ContentTree::removeResource(const QModelIndex &index) {
    if(index.isValid()) {
        QObject *item = getObject(index);
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

QModelIndex ContentTree::setNewAsset(const TString &name, const TString &source, bool directory) {
    if(!name.isEmpty()) {
        Url url(name);

        QObject *parent = m_rootItem->findChild<QObject *>(url.absoluteDir().data());
        if(parent == nullptr) {
            parent = m_content;
        }

        m_newAsset->setParent(parent);
        m_newAsset->setObjectName(name.data());
        m_newAsset->setProperty(gImport, source.data());
        m_newAsset->setProperty(gIcon, (directory) ? m_folder : AssetConverterSettings::documentIcon(Url(source).suffix()));
    } else {
        m_newAsset->setParent(nullptr);
    }

    emit layoutAboutToBeChanged();
    emit layoutChanged();

    return getIndex(m_newAsset);
}

void ContentTree::update() {
    TString contentPath(ProjectSettings::instance()->contentPath());
    QObject *parent = m_rootItem->findChild<QObject *>(contentPath.data());
    if(parent == nullptr) {
        parent = m_content;
    }
    clean(parent);

    AssetManager *mgr = AssetManager::instance();

    StringList list = File::list(contentPath);
    for(TString &path : list) {
        Url info(path);
        if(info.suffix() == gMetaExt) {
            continue;
        }

        TString parentDir = info.absoluteDir();
        parent = m_rootItem->findChild<QObject *>(parentDir.data());
        if(parent == nullptr) {
            parent = m_content;
        }

        QObject *item = parent->findChild<QObject *>(path.data());
        if(!item) {
            item = new QObject(parent);
            item->setObjectName(path.data());
            if(!File::isDir(path)) {
                item->setProperty(gType, mgr->assetTypeName(path).data());
                AssetConverterSettings *settings = mgr->fetchSettings(path);
                item->setProperty(gIcon, settings->icon(settings->destination()));
            } else {
                item->setProperty(gIcon, m_folder);
            }
            addItem(item);
        }
    }

    emit layoutAboutToBeChanged();
    emit layoutChanged();
}

void ContentTree::clean(QObject *parent) {
    foreach(QObject *it, parent->children()) {
        TString path = it->objectName().toStdString();
        clean(it);
        if(!File::exists(path)) {
            m_items.remove(reinterpret_cast<quintptr>(it));
            it->setParent(nullptr);
            it->deleteLater();
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
            QObject *item = getObject(index);
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
