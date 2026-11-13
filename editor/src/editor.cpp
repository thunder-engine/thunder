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
#include <editor.h>

#include "projectsettings.h"
#include "editorsettings.h"
#include "pluginmanager.h"

#include "legacydocumentmodel.h"
#include "editor/legacyeditorgadget.h"
#include "editor/legacyasseteditor.h"

#include "editor/editorgadget.h"
#include "editor/asseteditor.h"
#include "editor/documentmodel.h"
#include "editor/assetmanager.h"

#include <url.h>

#include <algorithm>

/*!
    \module Editor

    \title Thunder Engine Editor API's

    \brief Contains Editor management classes.
*/

/*!
    \class Editor
    \brief Provides access to the editor's documents, gadgets, settings, and plugins.
    \inmodule Editor

    The Editor class coordinates the main editor services and the currently
    opened asset editors.
*/

std::list<LegacyEditorGadget *> Editor::s_legacyGadgets;
std::list<EditorGadget *> Editor::s_gadgets;

LegacyAssetEditor *Editor::s_legacyCurrentEditor = nullptr;
LegacyDocumentModel *Editor::s_legacyDocumentModel = nullptr;
AssetEditor *Editor::s_currentEditor = nullptr;
DocumentModel *Editor::s_documentModel = nullptr;

EditorSettings *Editor::s_editorSettings = nullptr;
ProjectSettings *Editor::s_projectSettings = nullptr;
AssetManager *Editor::s_assetManager = nullptr;
PluginManager *Editor::s_pluginManager = nullptr;

TString Editor::m_binPath;

Editor::Editor() {

}

Editor::~Editor() {

}

void Editor::init(int argc, char *argv[]) {
    if(argc > 0) {
        Url url(argv[0]);
        m_binPath = url.absoluteDir();
    }

    Editor::plugins()->init(&Engine::instance());
}

void Editor::initDocumentSystem(bool useLegacy) {
    if(useLegacy) {
        s_legacyDocumentModel = new LegacyDocumentModel;

        for(auto &it : Editor::plugins()->extensions("legacygadget")) {
            addGadget(reinterpret_cast<LegacyEditorGadget *>(Editor::plugins()->getPluginObject(it)));
        }
    } else {
        s_documentModel = new DocumentModel;
        for(auto &it : Editor::plugins()->extensions("gadget")) {
            addGadget(reinterpret_cast<EditorGadget *>(Editor::plugins()->getPluginObject(it)));
        }
    }
}

void Editor::backup() {
    if(s_documentModel) {
        for(AssetEditor *editor : s_documentModel->documents()) {
            editor->backup();
        }
    }
    if(s_legacyDocumentModel) {
        for(auto it : s_legacyDocumentModel->documents()) {
            it->backup();
        }
    }
}

void Editor::restore() {
    if(s_documentModel) {
        for(AssetEditor *editor : s_documentModel->documents()) {
            editor->restore();
        }
    }
    if(s_legacyDocumentModel) {
        for(auto it : s_legacyDocumentModel->documents()) {
            it->restore();
        }
    }
}

void Editor::addEditor(AssetEditor *editor) {
    if(s_documentModel) {
        s_documentModel->addEditor(editor);
    }
}

void Editor::addEditor(LegacyAssetEditor *editor) {
    if(s_legacyDocumentModel) {
        s_legacyDocumentModel->addEditor(editor);
    }
}

bool Editor::closeEditor(AssetEditor *editor) {
    if(!s_documentModel || !s_documentModel->closeFile(editor)) {
        return false;
    }
    if(s_currentEditor == editor) {
        setCurrentEditor(static_cast<LegacyAssetEditor *>(nullptr));
        setCurrentEditor(static_cast<AssetEditor *>(nullptr));
    }
    return true;
}

void Editor::closeEditor(LegacyAssetEditor *editor) {
    if(s_legacyDocumentModel) {
        s_legacyDocumentModel->closeFile(editor);
    }
}

AssetEditor *Editor::openAssetFile(const TString &path) {
    AssetEditor *editor = s_documentModel ? s_documentModel->openFile(path) : nullptr;
    if(editor) {
        setCurrentEditor(editor);
    }
    return editor;
}

LegacyAssetEditor *Editor::legacyOpenFile(const TString &path) {
    if(s_legacyDocumentModel) {
        return s_legacyDocumentModel->openFile(path);
    }
    return nullptr;
}

AssetEditor *Editor::currentEditor() {
    return s_documentModel ? s_documentModel->currentEditor() : s_currentEditor;
}

void Editor::setCurrentEditor(AssetEditor *editor) {
    s_currentEditor = editor;
    if(s_documentModel) {
        s_documentModel->setCurrentEditor(editor);
    }
    for(EditorGadget *gadget : s_gadgets) {
        gadget->setCurrentEditor(editor);
    }
}

LegacyAssetEditor *Editor::legacyCurrentEditor() {
    return s_legacyCurrentEditor;
}

void Editor::setCurrentEditor(LegacyAssetEditor *editor) {
    s_legacyCurrentEditor = editor;
    foreach(LegacyEditorGadget *it, s_legacyGadgets) {
        it->setCurrentEditor(editor);
    }
}

std::list<AssetEditor *> Editor::documents() {
    return s_documentModel ? s_documentModel->documents() : std::list<AssetEditor *>();
}

std::list<LegacyAssetEditor *> Editor::legacyDocuments() {
    if(s_legacyDocumentModel) {
        return s_legacyDocumentModel->documents();
    }
    return std::list<LegacyAssetEditor *>();
}

void Editor::addGadget(EditorGadget *gadget) {
    if(gadget && std::find(s_gadgets.begin(), s_gadgets.end(), gadget) == s_gadgets.end()) {
        s_gadgets.push_back(gadget);
        gadget->setCurrentEditor(s_currentEditor);

        if(s_documentModel) {
            connect(s_documentModel, _SIGNAL(updated()), gadget, _SLOT(onUpdated()));
            connect(s_documentModel, _SIGNAL(selectionChanged()), gadget, _SLOT(onSelectionChanged()));
        }
    }
}

void Editor::addGadget(LegacyEditorGadget *gadget) {
    if(gadget) {
        s_legacyGadgets.push_back(gadget);

        QObject::connect(s_legacyDocumentModel, &LegacyDocumentModel::updated, gadget, &LegacyEditorGadget::onUpdated);
        QObject::connect(s_legacyDocumentModel, &LegacyDocumentModel::selectionChanaged, gadget, &LegacyEditorGadget::onSelectionChanged);

        QObject::connect(gadget, &LegacyEditorGadget::objectsSelected, s_legacyDocumentModel, &LegacyDocumentModel::selectionChanaged);
    }
}

std::list<LegacyEditorGadget *> Editor::legacyGadgets() {
    return s_legacyGadgets;
}

EditorSettings *Editor::settings() {
    if(s_editorSettings == nullptr) {
        s_editorSettings = new EditorSettings;
    }
    return s_editorSettings;
}

ProjectSettings *Editor::project() {
    if(s_projectSettings == nullptr) {
        s_projectSettings = new ProjectSettings;
    }
    return s_projectSettings;
}

AssetManager *Editor::assets() {
    if(s_assetManager == nullptr) {
        s_assetManager = new AssetManager;
    }
    return s_assetManager;
}

PluginManager *Editor::plugins() {
    if(s_pluginManager == nullptr) {
        s_pluginManager = new PluginManager;
    }
    return s_pluginManager;
}

TString Editor::applicationDirPath() {
    return m_binPath;
}
