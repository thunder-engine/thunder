#include "documentmodel.h"

#include <QFileInfo>
#include <QDir>
#include <QEvent>

#include "assetmanager.h"

#include <editor/projectmanager.h>
#include <editor/pluginmanager.h>

#include "editors/animationedit/animationedit.h"

DocumentModel::DocumentModel() {
    addEditor(new AnimationEdit);

    for(auto &it : PluginManager::instance()->extensions("editor")) {
        AssetEditor *editor = reinterpret_cast<AssetEditor *>(PluginManager::instance()->getPluginObject(it));
        addEditor(editor);
    }
}

DocumentModel::~DocumentModel() {
    auto it = m_documents.begin();
    while(it != m_documents.end()) {
        if(!(*it)->isSingleInstance()) {
            (*it)->deleteLater();
        }
        ++it;
    }
    m_documents.clear();

    for(auto &it : m_editors) {
        delete it;
    }
    m_editors.clear();
}

void DocumentModel::addEditor(AssetEditor *editor) {
    for(auto &it : editor->suffixes()) {
        m_editors[it] = editor;
    }
    editor->installEventFilter(this);
    connect(editor, &AssetEditor::itemsSelected, this, &DocumentModel::itemsSelected);
    connect(editor, &AssetEditor::itemsUpdated, this, &DocumentModel::itemsUpdated);

    connect(editor, &AssetEditor::dropAsset, this, &DocumentModel::onLoadAsset);
}

void DocumentModel::newFile(AssetEditor *editor) {
    if(editor->checkSave()) {
        closeFile(editor);
        editor->onNewAsset();
    }
}

AssetEditor *DocumentModel::openFile(const QString &path) {
    QFileInfo info(path);
    QDir dir(ProjectManager::instance()->contentPath());
    AssetConverterSettings *settings = AssetManager::instance()->fetchSettings(dir.absoluteFilePath(info.filePath()));

    AssetEditor *editor = nullptr;

    for(auto &it : m_documents) {
        for(auto doc : it->documentsSettings()) {
            if(doc == settings) {
                editor = it;
                return editor;
            }
        }
    }

    auto e = m_editors.find(info.suffix().toLower());
    if(e != m_editors.end()) {
        editor = e.value();
        if(!editor->isSingleInstance()) {
            AssetEditor *instance = editor->createInstance();
            instance->installEventFilter(this);
            editor = instance;
        } else { // Single instance! Need to close previous document
            if(editor->checkSave()) {
                closeFile(editor);
            }
        }
    }

    if(editor) {
        editor->loadAsset(settings);
        if(!m_documents.contains(editor)) {
            m_documents.push_back(editor);
        }
    }

    return editor;
}

void DocumentModel::closeFile(AssetEditor *editor) {
    for(auto it = m_documents.begin(); it != m_documents.end();) {
        if(*it == editor) {
            if(!editor->isSingleInstance()) {
                editor->deleteLater();
            }
            it = m_documents.erase(it);
        } else {
            ++it;
        }
    }
}

void DocumentModel::onLoadAsset(QString path) {
    openFile(path);
}

QList<AssetEditor *> DocumentModel::documents() {
    return m_documents;
}

bool DocumentModel::eventFilter(QObject *obj, QEvent *event) {
    if(event->type() == QEvent::Close) {
        AssetEditor *editor = static_cast<AssetEditor *>(obj);
        if(!editor->checkSave()) {
            event->ignore();
        } else {
            closeFile(editor);
        }
        return true;
    }
    return QObject::eventFilter(obj, event);
}
