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
#include "editor/assetmanager.h"

#include <url.h>

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

std::list<LegacyEditorGadget *> Editor::s_gadgets;

LegacyAssetEditor *Editor::s_currentEditor = nullptr;

EditorSettings *Editor::s_editorSettings = nullptr;
ProjectSettings *Editor::s_projectSettings = nullptr;
AssetManager *Editor::s_assetManager = nullptr;
PluginManager *Editor::s_pluginManager = nullptr;

LegacyDocumentModel *Editor::s_documentModel = nullptr;

TString Editor::m_binPath;

Editor::Editor(int argc, char *argv[]) {
    if(argc > 0) {
        Url url(argv[0]);
        m_binPath = url.absoluteDir();
    }
}

Editor::~Editor() {

}

void Editor::init() {
    s_documentModel = new LegacyDocumentModel;

    for(auto &it : Editor::plugins()->extensions("gadget")) {
        addGadget(reinterpret_cast<LegacyEditorGadget *>(Editor::plugins()->getPluginObject(it)));
    }
}

void Editor::backup() {
    if(s_documentModel) {
        for(auto it : s_documentModel->documents()) {
            it->backup();
        }
    }
}

void Editor::restore() {
    if(s_documentModel) {
        for(auto it : s_documentModel->documents()) {
            it->restore();
        }
    }
}

void Editor::addEditor(LegacyAssetEditor *editor) {
    if(s_documentModel) {
        s_documentModel->addEditor(editor);
    }
}

void Editor::closeEditor(LegacyAssetEditor *editor) {
    if(s_documentModel) {
        s_documentModel->closeFile(editor);
    }
}

LegacyAssetEditor *Editor::openFile(const TString &path) {
    if(s_documentModel) {
        return s_documentModel->openFile(path);
    }
    return nullptr;
}

LegacyAssetEditor *Editor::currentEditor() {
    return s_currentEditor;
}

void Editor::setCurrentEditor(LegacyAssetEditor *editor) {
    foreach(auto it, s_gadgets) {
        if(s_currentEditor) {
            QObject::disconnect(it, &LegacyEditorGadget::updated, s_currentEditor, &LegacyAssetEditor::onUpdated);
            QObject::disconnect(it, &LegacyEditorGadget::objectsSelected, s_currentEditor, &LegacyAssetEditor::onObjectsSelected);

            QObject::disconnect(s_currentEditor, &LegacyAssetEditor::objectsChanged, it, &LegacyEditorGadget::onObjectsChanged);
        }

        QObject::connect(it, &LegacyEditorGadget::updated, editor, &LegacyAssetEditor::onUpdated);
        QObject::connect(it, &LegacyEditorGadget::objectsSelected, editor, &LegacyAssetEditor::onObjectsSelected);

        QObject::connect(editor, &LegacyAssetEditor::objectsChanged, it, &LegacyEditorGadget::onObjectsChanged);

        it->setCurrentEditor(editor);
    }

    s_currentEditor = editor;
}

std::list<LegacyAssetEditor *> Editor::documents() {
    if(s_documentModel) {
        return s_documentModel->documents();
    }
    return std::list<LegacyAssetEditor *>();
}

void Editor::addGadget(LegacyEditorGadget *gadget) {
    if(gadget) {
        s_gadgets.push_back(gadget);

        QObject::connect(s_documentModel, &LegacyDocumentModel::updated, gadget, &LegacyEditorGadget::onUpdated);
        QObject::connect(s_documentModel, &LegacyDocumentModel::selectionChanaged, gadget, &LegacyEditorGadget::onSelectionChanged);

        QObject::connect(gadget, &LegacyEditorGadget::objectsSelected, s_documentModel, &LegacyDocumentModel::selectionChanaged);
    }
}

std::list<LegacyEditorGadget *> Editor::gadgets() {
    return s_gadgets;
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
