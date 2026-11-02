/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#include <editor/documentmodel.h>

#include <editor/asseteditor.h>
#include <editor/assetmanager.h>
#include <log.h>
#include <aprocess.h>
#include <url.h>

#include <algorithm>

DocumentModel::DocumentModel() :
        m_currentEditor(nullptr) {
}

DocumentModel::~DocumentModel() {
    for(AssetEditor *editor : m_documents) {
        if(!editor->isSingleInstance()) {
            editor->deleteLater();
        }
    }
}

void DocumentModel::addEditor(AssetEditor *editor) {
    if(!editor) {
        aWarning() << "Unable to register a null asset editor.";
        return;
    }

    for(const TString &suffix : editor->suffixes()) {
        TString normalized = suffix.toLower();
        if(normalized.startsWith(".")) {
            normalized = normalized.mid(1, normalized.length() - 1);
        }
        if(normalized.isEmpty()) {
            aWarning() << "Ignoring an empty asset-editor suffix.";
            continue;
        }
        m_editors[normalized] = editor;
    }

    connectEditor(editor);
}

AssetEditor *DocumentModel::openFile(const TString &path) {
    AssetConverterSettings *settings = Editor::assets()->fetchSettings(path);
    if(!settings) {
        aWarning() << "Unable to load asset settings for:" << path;
        return nullptr;
    }

    Url info(path);
    const TString suffix = info.suffix().toLower();
    auto it = m_editors.find(suffix);
    if(it == m_editors.end()) {
        if(!Process::openUrl(settings->source())) {
            aWarning() << "No asset editor is registered for:" << suffix << "and the asset could not be opened externally:" << path;
        }
        return nullptr;
    }

    for(AssetEditor *document : m_documents) {
        const auto &opened = document->openedDocuments();
        if(std::find(opened.begin(), opened.end(), settings) != opened.end()) {
            setCurrentEditor(document);
            return document;
        }
    }

    AssetEditor *editor = it->second;
    if(editor->isSingleInstance()) {
        if(!editor->openedDocuments().empty()) {
            if(!editor->checkSave()) {
                aInfo() << "Opening cancelled because the current document has unsaved changes:" << path;
                return nullptr;
            }
            closeFile(editor);
        }
    } else {
        editor = editor->createInstance();
        if(!editor) {
            aError() << "Unable to create an instance of the asset editor for:" << suffix;
            return nullptr;
        }
        connectEditor(editor);
    }

    editor->loadAsset(settings);
    if(std::find(m_documents.begin(), m_documents.end(), editor) == m_documents.end()) {
        m_documents.push_back(editor);
    }
    setCurrentEditor(editor);

    return editor;
}

bool DocumentModel::closeFile(AssetEditor *editor) {
    auto it = std::find(m_documents.begin(), m_documents.end(), editor);
    if(it == m_documents.end()) {
        return false;
    }
    if(!editor->checkSave()) {
        aInfo() << "Closing cancelled because the document has unsaved changes.";
        return false;
    }

    m_documents.erase(it);
    if(m_currentEditor == editor) {
        setCurrentEditor(nullptr);
    }
    if(!editor->isSingleInstance()) {
        editor->deleteLater();
    }
    return true;
}

std::list<AssetEditor *> DocumentModel::documents() const {
    return m_documents;
}

AssetEditor *DocumentModel::currentEditor() const {
    return m_currentEditor;
}

void DocumentModel::setCurrentEditor(AssetEditor *editor) {
    if(m_currentEditor == editor) {
        return;
    }
    m_currentEditor = editor;
    updated();
    selectionChanged();
}

void DocumentModel::updated() {
    emitSignal(_SIGNAL(updated()));
}

void DocumentModel::selectionChanged() {
    emitSignal(_SIGNAL(selectionChanged()));
}

void DocumentModel::onEditorUpdated() {
    updated();
}

void DocumentModel::onEditorSelectionChanged() {
    selectionChanged();
}

void DocumentModel::onEditorDestroyed() {
    auto *editor = dynamic_cast<AssetEditor *>(sender());
    if(!editor) {
        return;
    }

    m_documents.remove(editor);
    m_connectedEditors.remove(editor);
    for(auto it = m_editors.begin(); it != m_editors.end();) {
        if(it->second == editor) {
            it = m_editors.erase(it);
        } else {
            ++it;
        }
    }
    if(m_currentEditor == editor) {
        setCurrentEditor(nullptr);
    }
}

void DocumentModel::onLoadAsset(const TString &path) {
    Editor::openAssetFile(path);
}

void DocumentModel::connectEditor(AssetEditor *editor) {
    if(std::find(m_connectedEditors.begin(), m_connectedEditors.end(), editor) != m_connectedEditors.end()) {
        return;
    }

    m_connectedEditors.push_back(editor);
    connect(editor, _SIGNAL(updated()), this, _SLOT(onEditorUpdated()));
    connect(editor, _SIGNAL(selectionChanged()), this, _SLOT(onEditorSelectionChanged()));
    connect(editor, _SIGNAL(editorClosing()), this, _SLOT(onEditorDestroyed()));
    connect(editor, _SIGNAL(dropAsset(TString)), this, _SLOT(onLoadAsset(TString)));
}
