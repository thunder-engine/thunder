#include "documentmodel.h"

#include <QFileInfo>
#include <QDir>
#include <QEvent>

#include <editor/assetmanager.h>
#include <editor/asseteditor.h>
#include <editor/projectsettings.h>
#include <editor/pluginmanager.h>

DocumentModel::DocumentModel() {
    for(auto &it : PluginManager::instance()->extensions("editor")) {
        addEditor(reinterpret_cast<AssetEditor *>(PluginManager::instance()->getPluginObject(it)));
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
        delete it.second;
    }
    m_editors.clear();
}

void DocumentModel::addEditor(AssetEditor *editor) {
    for(auto &it : editor->suffixes()) {
        m_editors[it] = editor;
    }
    editor->installEventFilter(this);
    connect(editor, &AssetEditor::itemsSelected, this, &DocumentModel::itemsSelected);
    connect(editor, &AssetEditor::objectsSelected, this, &DocumentModel::objectsSelected);
    connect(editor, &AssetEditor::updated, this, &DocumentModel::updated);

    connect(editor, &AssetEditor::dropAsset, this, &DocumentModel::onLoadAsset);
}

void DocumentModel::newFile(AssetEditor *editor) {
    if(editor->checkSave()) {
        closeFile(editor);
        editor->onNewAsset();
    }
}

AssetEditor *DocumentModel::openFile(const QString &path) {    
    QDir dir(ProjectSettings::instance()->contentPath());
    AssetConverterSettings *settings = AssetManager::instance()->fetchSettings(dir.absoluteFilePath(path));

    AssetEditor *editor = nullptr;

    // Check if document already opened
    for(auto &it : m_documents) {
        foreach(auto doc, it->openedDocuments()) {
            if(doc == settings) {
                editor = it;
                editor->loadAsset(settings);
                return editor;
            }
        }
    }

    QFileInfo info(path);

    auto e = m_editors.find(info.suffix().toLower());
    if(e != m_editors.end()) {
        editor = e->second;
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

    if(editor && settings) {
        editor->loadAsset(settings);
        if(std::find(m_documents.begin(), m_documents.end(), editor) == m_documents.end()) {
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

std::list<AssetEditor *> DocumentModel::documents() {
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
