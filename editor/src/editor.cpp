#include <editor.h>

#include "projectsettings.h"
#include "editorsettings.h"
#include "pluginmanager.h"

#include "documentmodel.h"

#include "editor/editorgadget.h"
#include "editor/asseteditor.h"
#include "editor/assetmanager.h"

/*!
    \module Editor

    \title Thunder Engine Editor API's

    \brief Contains Editor management classes.
*/

std::list<EditorGadget *> Editor::s_gadgets;

AssetEditor *Editor::s_currentEditor = nullptr;

EditorSettings *Editor::s_editorSettings = nullptr;
ProjectSettings *Editor::s_projectSettings = nullptr;
AssetManager *Editor::s_assetManager = nullptr;
PluginManager *Editor::s_pluginManager = nullptr;

DocumentModel *Editor::s_documentModel = nullptr;

Editor::Editor() {

}

Editor::~Editor() {

}

void Editor::init() {
    s_documentModel = new DocumentModel;

    for(auto &it : Editor::plugins()->extensions("gadget")) {
        addGadget(reinterpret_cast<EditorGadget *>(Editor::plugins()->getPluginObject(it)));
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

void Editor::addEditor(AssetEditor *editor) {
    if(s_documentModel) {
        s_documentModel->addEditor(editor);
    }
}

void Editor::closeEditor(AssetEditor *editor) {
    if(s_documentModel) {
        s_documentModel->closeFile(editor);
    }
}

AssetEditor *Editor::openFile(const TString &path) {
    if(s_documentModel) {
        return s_documentModel->openFile(path);
    }
    return nullptr;
}

AssetEditor *Editor::currentEditor() {
    return s_currentEditor;
}

void Editor::setCurrentEditor(AssetEditor *editor) {
    foreach(auto it, s_gadgets) {
        if(s_currentEditor) {
            QObject::disconnect(it, &EditorGadget::updated, s_currentEditor, &AssetEditor::onUpdated);
            QObject::disconnect(it, &EditorGadget::objectsSelected, s_currentEditor, &AssetEditor::onObjectsSelected);

            QObject::disconnect(s_currentEditor, &AssetEditor::objectsChanged, it, &EditorGadget::onObjectsChanged);
        }

        QObject::connect(it, &EditorGadget::updated, editor, &AssetEditor::onUpdated);
        QObject::connect(it, &EditorGadget::objectsSelected, editor, &AssetEditor::onObjectsSelected);

        QObject::connect(editor, &AssetEditor::objectsChanged, it, &EditorGadget::onObjectsChanged);

        it->setCurrentEditor(editor);
    }

    s_currentEditor = editor;
}

std::list<AssetEditor *> Editor::documents() {
    if(s_documentModel) {
        return s_documentModel->documents();
    }
    return std::list<AssetEditor *>();
}

void Editor::addGadget(EditorGadget *gadget) {
    if(gadget) {
        s_gadgets.push_back(gadget);

        QObject::connect(s_documentModel, &DocumentModel::updated, gadget, &EditorGadget::onUpdated);
        QObject::connect(s_documentModel, &DocumentModel::selectionChanaged, gadget, &EditorGadget::onSelectionChanged);

        QObject::connect(gadget, &EditorGadget::objectsSelected, s_documentModel, &DocumentModel::selectionChanaged);
    }
}

std::list<EditorGadget *> Editor::gadgets() {
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
