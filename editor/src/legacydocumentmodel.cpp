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
#include "legacydocumentmodel.h"

#include <QEvent>

#include <editor/assetmanager.h>
#include <editor/legacyasseteditor.h>
#include <editor/assetconverter.h>
#include <editor/projectsettings.h>
#include <editor/legacypluginmanager.h>

#include <url.h>
#include <aprocess.h>

/*!
    \class LegacyDocumentModel
    \brief Manages the asset documents opened in the editor.
    \inmodule Editor

    LegacyDocumentModel tracks open asset editors and emits notifications when the
    document set or selection changes.
*/

LegacyDocumentModel::LegacyDocumentModel() {
    for(auto &it : Editor::plugins()->extensions("editor")) {
        addEditor(reinterpret_cast<LegacyAssetEditor *>(Editor::plugins()->getPluginObject(it)));
    }
}

LegacyDocumentModel::~LegacyDocumentModel() {
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

void LegacyDocumentModel::addEditor(LegacyAssetEditor *editor) {
    for(auto &it : editor->suffixes()) {
        m_editors[it] = editor;
    }
    editor->installEventFilter(this);
    connect(editor, &LegacyAssetEditor::selectionChanged, this, &LegacyDocumentModel::selectionChanaged);
    connect(editor, &LegacyAssetEditor::updated, this, &LegacyDocumentModel::updated);

    connect(editor, &LegacyAssetEditor::dropAsset, this, &LegacyDocumentModel::onLoadAsset);
}

LegacyAssetEditor *LegacyDocumentModel::openFile(const TString &path) {
    AssetConverterSettings *settings = Editor::assets()->fetchSettings(path);

    LegacyAssetEditor *editor = nullptr;

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

    Url info(path);

    auto e = m_editors.find(info.suffix().toLower());
    if(e != m_editors.end()) {
        editor = e->second;
        if(!editor->isSingleInstance()) {
            LegacyAssetEditor *instance = editor->createInstance();
            instance->installEventFilter(this);
            editor = instance;
        } else { // Single instance! Need to close previous document
            if(editor->checkSave()) {
                closeFile(editor);
            }
        }
    }

    if(settings) {
        if(editor) {
            editor->loadAsset(settings);
            if(std::find(m_documents.begin(), m_documents.end(), editor) == m_documents.end()) {
                m_documents.push_back(editor);
            }
        } else {
            Process::openUrl(settings->source());
        }
    }

    return editor;
}

void LegacyDocumentModel::closeFile(LegacyAssetEditor *editor) {
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

void LegacyDocumentModel::onLoadAsset(QString path) {
    openFile(path.toStdString());
}

std::list<LegacyAssetEditor *> LegacyDocumentModel::documents() {
    return m_documents;
}

bool LegacyDocumentModel::eventFilter(QObject *obj, QEvent *event) {
    if(event->type() == QEvent::Close) {
        LegacyAssetEditor *editor = static_cast<LegacyAssetEditor *>(obj);
        if(!editor->checkSave()) {
            event->ignore();
        } else {
            closeFile(editor);
        }
        return true;
    }
    return QObject::eventFilter(obj, event);
}
