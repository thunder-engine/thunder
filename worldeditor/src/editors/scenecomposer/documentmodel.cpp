#include "documentmodel.h"

#include <QFileInfo>
#include <QDir>

#include "assetmanager.h"
#include "projectmanager.h"

#include "editors/textedit/textedit.h"

#include "editors/textureedit/textureedit.h"
#include "editors/materialedit/materialedit.h"
#include "editors/meshedit/meshedit.h"
#include "editors/particleedit/particleedit.h"
#include "editors/animationedit/animationedit.h"

DocumentModel::DocumentModel(Engine *engine) {
    addEditor(IConverter::ContentTexture, new TextureEdit(engine));
    addEditor(IConverter::ContentMaterial, new MaterialEdit(engine));
    addEditor(IConverter::ContentPrefab, new MeshEdit(engine));
    addEditor(IConverter::ContentEffect, new ParticleEdit(engine));
    addEditor(IConverter::ContentAnimationStateMachine, new AnimationEdit(engine));
}

DocumentModel::~DocumentModel() {
    foreach(auto it, m_Editors) {
        delete it;
    }
    m_Editors.clear();
}

void DocumentModel::addEditor(uint8_t type, IAssetEditor *editor) {
    m_Editors[type] = editor;
}

IAssetEditor *DocumentModel::openFile(const QString &path) {
    auto it = m_Documents.find(path);
    if(it != m_Documents.end()) {
        return it.value();
    }

    QFileInfo info(path);
    int32_t type = AssetManager::instance()->resourceType(info);

    QDir dir(ProjectManager::instance()->contentPath());
    IConverterSettings *settings = AssetManager::instance()->fetchSettings(dir.absoluteFilePath(info.filePath()));

    IAssetEditor *editor = nullptr;
    auto e = m_Editors.find(type);
    if(e != m_Editors.end()) {
        editor = e.value();
    } else {
        switch(type) {
            case IConverter::ContentCode: {
                editor = new TextEdit(nullptr);
            } break;
            default: break;
        }
    }

    editor->loadAsset(settings);
    m_Documents[path] = editor;

    return editor;
}

void DocumentModel::closeFile(const QString &path) {
    auto it = m_Documents.find(path);
    if(it != m_Documents.end()) {
        QFileInfo info(path);
        int32_t type = AssetManager::instance()->resourceType(info);
        switch(type) {
            case IConverter::ContentCode: {
                TextEdit *editor = dynamic_cast<TextEdit *>(it.value());
                if(editor) {
                    delete editor;
                }
            } break;
            default: break;
        }

        m_Documents.erase(it);
    }
}

void DocumentModel::saveAll() {
    foreach(auto it, m_Documents) {
        if(it->isModified()) {
            //it->saveFile();
        }
    }
}

QVariant DocumentModel::data(const QModelIndex &index, int role) const {
    switch(role) {
        case Qt::EditRole:
        case Qt::ToolTipRole:
        case Qt::DisplayRole: {
            QString key = m_Documents.keys().at(index.row());
            QFileInfo info(key);
            if(index.column() == 0) {
                return info.fileName();
            }
        } break;
        default: break;
    }

    return QVariant();
}

QVariant DocumentModel::headerData(int, Qt::Orientation orientation, int role) const {
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return "";
    }
    return QVariant();
}

int DocumentModel::columnCount(const QModelIndex &) const {
    return 2;
}

QModelIndex DocumentModel::index(int row, int column, const QModelIndex &) const {
    if(row >= m_Documents.size()) {
        return QModelIndex();
    }
    return createIndex(row, column, nullptr);
}

QModelIndex DocumentModel::parent(const QModelIndex &) const {
    return QModelIndex();
}

int DocumentModel::rowCount(const QModelIndex &parent) const {
    if(parent.isValid()) {
        return 0;
    }
    return m_Documents.size();
}
