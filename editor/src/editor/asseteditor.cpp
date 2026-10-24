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
#include "asseteditor.h"

#include <filedialog.h>
#include <log.h>

#include <editor/assetmanager.h>
#include <editor/projectsettings.h>



AssetEditor::AssetEditor() :
        m_undoRedo(new UndoStack) {
}

AssetEditor::~AssetEditor() {
    delete m_undoRedo;
}

StringList AssetEditor::componentGroups() const {
    return StringList();
}

Object::ObjectList AssetEditor::selected() const {
    return Object::ObjectList();
}

VariantMap AssetEditor::saveState() {
    return VariantMap();
}

void AssetEditor::restoreState(const VariantMap &data) {
    A_UNUSED(data);
}

void AssetEditor::backup() {
}

void AssetEditor::restore() {
}

void AssetEditor::onActivated() {
}

void AssetEditor::onNewAsset() {
    m_settings.clear();
    m_undoRedo->clear();
}

void AssetEditor::onOpenAsset() {
    if(!Editor::project() || !Editor::assets()) {
        aError() << "Unable to open an asset before the project is initialized.";
        return;
    }

    FileDialog dialog;
    dialog.setMode(FileDialog::OpenFile);
    dialog.setWindowTitle(TString("Open ") + assetType());

    StringList filters;
    for(const TString &suffix : suffixes()) {
        filters.push_back(TString("*.") + suffix);
    }
    dialog.addFilter(assetType(), filters);
    dialog.setDirectory(Editor::project()->contentPath());

    if(dialog.exec()) {
        TString path = dialog.getSelectedFile();
        if(!path.isEmpty()) {
            AssetConverterSettings *settings = Editor::assets()->fetchSettings(path);
            if(settings) {
                loadAsset(settings);
            } else {
                aWarning() << "Unable to load asset settings for" << path;
            }
        }
    }
}

void AssetEditor::onSave() {
    if(!m_settings.empty() && m_settings.front()) {
        saveAsset(m_settings.front()->source());
    }
}

void AssetEditor::onSaveAs() {
    saveAsset();
}

void AssetEditor::onObjectCreate(const TString &type) {
    A_UNUSED(type);
}

void AssetEditor::onObjectsSelected(Object::ObjectList objects, bool force) {
    A_UNUSED(objects);
    A_UNUSED(force);
}

void AssetEditor::changeParent(const Object::ObjectList &objects, Object *parent, int position) {
    A_UNUSED(objects);
    A_UNUSED(parent);
    A_UNUSED(position);
}

void AssetEditor::onDropAsset(const TString &path, bool additive) {
    A_UNUSED(path);
    A_UNUSED(additive);
}

void AssetEditor::onSelectionDeleted() {
}

void AssetEditor::onUpdated() {
    updated();
}

void AssetEditor::onObjectsChanged(const Object::ObjectList &objects, const TString &property, const Variant &value) {
    A_UNUSED(objects);
    A_UNUSED(property);
    A_UNUSED(value);
}

std::list<AssetConverterSettings *> &AssetEditor::openedDocuments() {
    return m_settings;
}

UndoStack *AssetEditor::undoRedo() const {
    return m_undoRedo;
}

bool AssetEditor::isModified() const {
    return !m_undoRedo->isClean();
}

void AssetEditor::cleanModified() const {
    m_undoRedo->setClean();
}

void AssetEditor::saveAsset(const TString &path) {
    A_UNUSED(path);
    cleanModified();
}

void AssetEditor::updated() {
    emitSignal(_SIGNAL(updated()));
}

void AssetEditor::objectsHierarchyChanged(Object *root) {
    emitSignal(_SIGNAL(objectsHierarchyChanged(Object *)), Variant::fromValue(root));
}

void AssetEditor::selectionChanged() {
    emitSignal(_SIGNAL(selectionChanged()));
}

void AssetEditor::objectsChanged(const Object::ObjectList &objects, const TString &property, const Variant &value) {
    //emitSignal(_SIGNAL(objectsChanged(Object::ObjectList,TString,Variant)), objects, property, value);
}

void AssetEditor::copyPasteChanged() {
    emitSignal(_SIGNAL(copyPasteChanged()));
}

void AssetEditor::dropAsset(const TString &path) {
    emitSignal(_SIGNAL(dropAsset(TString)), path);
}
