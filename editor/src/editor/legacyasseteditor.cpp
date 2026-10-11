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
#include "editor/legacyasseteditor.h"

#include <url.h>
#include <filedialog.h>
#include <message.h>

#include "editor/assetconverter.h"
#include "editor/projectsettings.h"
#include "editor/assetmanager.h"
#include "editor/undostack.h"

/*!
    \class LegacyAssetEditor
    \brief Provides the base interface for editors of engine assets.
    \inmodule Editor

    LegacyAssetEditor manages asset loading, saving, selection, editing actions, and
    undo and redo operations for a particular asset type.
*/

LegacyAssetEditor::LegacyAssetEditor() :
        m_undoRedo(new UndoStack) {

}

LegacyAssetEditor::~LegacyAssetEditor() {

}

/*!
    Opens a new asset in the editor and clears the current edit history.
*/
void LegacyAssetEditor::onNewAsset() {
    m_settings.clear();
    m_undoRedo->clear();
}

/*!
    Opens a file dialog and loads an asset selected by the user.
*/
void LegacyAssetEditor::onOpenAsset() {
    FileDialog dialog;

    dialog.setMode(FileDialog::OpenFile);
    dialog.setWindowTitle(TString("Save ") + assetType());

    StringList list;
    for(auto &it : suffixes()) {
        list.push_back(TString("*.") + it);
    }

    dialog.addFilter(assetType(), list);
    dialog.setDirectory(Editor::project()->contentPath());

    if(dialog.exec()) {
        TString path(dialog.getSelectedFile());
        if(!path.isEmpty()) {
            loadAsset(Editor::assets()->fetchSettings(path));
        }
    }
}

/*!
    Loads the asset described by \a settings.
*/
void LegacyAssetEditor::loadAsset(AssetConverterSettings *settings) {
    m_settings = { settings };
    m_undoRedo->clear();
}

/*!
    Loads editor state from \a data for an asset with the given \a suffix.
*/
void LegacyAssetEditor::loadData(const Variant &data, const TString &suffix) {
    Q_UNUSED(data)
    Q_UNUSED(suffix)
}

/*!
    Returns true if the editor can save an asset under a new name.
*/
bool LegacyAssetEditor::allowSaveAs() const {
    return true;
}

/*!
    Stores a temporary backup of the current editor state.
*/
void LegacyAssetEditor::backup() {

}

/*!
    Restores the editor state from the most recent backup.
*/
void LegacyAssetEditor::restore() {

}

void LegacyAssetEditor::saveAsset(const TString &path) {
    Q_UNUSED(path)

    cleanModified();
}

/*!
    Returns true when this editor accepts only one instance of its asset type.
*/
bool LegacyAssetEditor::isSingleInstance() const {
    return true;
}

/*!
    Returns whether the copy action is available for the current selection.
*/
bool LegacyAssetEditor::isCopyActionAvailable() const {
    return false;
}

/*!
    Returns whether the paste action is available for the current selection.
*/
bool LegacyAssetEditor::isPasteActionAvailable() const {
    return false;
}

/*!
    Creates another instance of this asset editor.
*/
LegacyAssetEditor *LegacyAssetEditor::createInstance() {
    return nullptr;
}

/*!
    Returns the asset suffixes supported by this editor.
*/
StringList LegacyAssetEditor::suffixes() const {
    return StringList();
}

/*!
    Returns the asset type handled by this editor.

    \fn TString LegacyAssetEditor::assetType() const
*/

/*!
    Returns the documents currently opened by this editor.
*/
std::list<AssetConverterSettings *> &LegacyAssetEditor::openedDocuments() {
    return m_settings;
}

/*!
    Returns the component groups supported by this editor.
*/
StringList LegacyAssetEditor::componentGroups() const {
    return StringList();
}

void LegacyAssetEditor::cleanModified() const {
    m_undoRedo->setClean();
}

bool LegacyAssetEditor::isModified() const {
    return !m_undoRedo->isClean();
}

/*!
    Activates the editor and updates its active state.
*/
void LegacyAssetEditor::onActivated() {

}

/*!
    Handles the cut action for the current selection.
*/
void LegacyAssetEditor::onCutAction() {

}

/*!
    Handles the copy action for the current selection.
*/
void LegacyAssetEditor::onCopyAction() {

}

/*!
    Handles the paste action for the current selection.
*/
void LegacyAssetEditor::onPasteAction() {

}

int LegacyAssetEditor::closeAssetDialog() {
    Message msgBox;
    msgBox.setIcon(Message::Question);
    msgBox.setText(tr("The asset has been modified.").toStdString());
    msgBox.setInformativeText(tr("Do you want to save your changes?").toStdString());
    msgBox.setStandardButtons(Message::Yes | Message::No | Message::Cancel);
    msgBox.setDefaultButton(Message::Cancel);

    return msgBox.exec();
}

/*!
    Returns true if it is safe to close the editor, prompting to save changes.
*/
bool LegacyAssetEditor::checkSave() {
    if(isModified()) {
        int result = closeAssetDialog();
        if(result == Message::Cancel) {
            return false;
        } else if(result == Message::Yes) {
            onSave();
        } else {
            cleanModified();
        }
    }
    return true;
}

/*!
    Saves the current asset.
*/
void LegacyAssetEditor::onSave() {
    if(!m_settings.empty()) {
        if(!m_settings.front()->source().isEmpty()) {
            saveAsset(m_settings.front()->source());
        } else if(allowSaveAs()) {
            onSaveAs();
        }
    }
}

/*!
    Saves the current asset under a new path selected by the user.
*/
void LegacyAssetEditor::onSaveAs() {
    if(m_settings.empty()) {
        return;
    }

    TString assetType(m_settings.front()->typeName());
    StringList list;
    for(auto &it : suffixes()) {
        list.push_back(TString("*.") + it);
    }

    FileDialog dialog;

    dialog.setMode(FileDialog::SaveFile);
    dialog.setWindowTitle(TString("Save ") + assetType);
    dialog.addFilter(assetType, list);

    dialog.setDirectory(Editor::project()->contentPath());

    if(dialog.exec()) {
        TString path(dialog.getSelectedFile());
        Url info(path);
        if(info.suffix().isEmpty()) {
            path += TString(".") + LegacyAssetEditor::suffixes().front();
        }
        saveAsset(path);
    }
}

/*!
    Handles creation of an object of the specified type.
*/
void LegacyAssetEditor::onObjectCreate(const TString &type) {
    A_UNUSED(type);
}

/*!
    Updates the editor selection with the supplied objects.
*/
void LegacyAssetEditor::onObjectsSelected(Object::ObjectList objects, bool force) {
    A_UNUSED(objects);
    A_UNUSED(force);
}

/*!
    Removes the currently selected objects from the editor.
*/
void LegacyAssetEditor::onSelectionDeleted() {

}

/*!
    Refreshes the editor after an external update.
*/
void LegacyAssetEditor::onUpdated() {

}

/*!
    Handles an asset dropped onto the editor.
*/
void LegacyAssetEditor::onDrop(QDropEvent *event) {
    A_UNUSED(event);
}

/*!
    Handles entry of a drag operation over the editor.
*/
void LegacyAssetEditor::onDragEnter(QDragEnterEvent *event) {
    A_UNUSED(event);
}

/*!
    Handles movement of a drag operation over the editor.
*/
void LegacyAssetEditor::onDragMove(QDragMoveEvent *event) {
    A_UNUSED(event);
}

/*!
    Handles leaving a drag operation from the editor.
*/
void LegacyAssetEditor::onDragLeave(QDragLeaveEvent *event) {
    A_UNUSED(event);
}

/*!
    Notifies the editor that properties changed on the supplied objects.
*/
void LegacyAssetEditor::onObjectsChanged(const Object::ObjectList &objects, const TString &property, const Variant &value) {
    A_UNUSED(objects);
    A_UNUSED(property);
    A_UNUSED(value);
}

QMenu *LegacyAssetEditor::hierarchyContextMenu(Object *object) {
    A_UNUSED(object);

    return nullptr;
}

/*!
    Changes the parent of the supplied objects.
*/
void LegacyAssetEditor::changeParent(const Object::ObjectList &objects, Object *parent, int position) {
    A_UNUSED(objects);
    A_UNUSED(parent);
    A_UNUSED(position);
}

/*!
    Returns the widget used to edit the selected object's properties.
*/
QWidget *LegacyAssetEditor::propertiesWidget() {
    return nullptr;
}

/*!
    Returns additional property action widgets for the selected object.
*/
std::list<QWidget *> LegacyAssetEditor::propertiesActionWidgets(Object *object, QWidget *parent) const {
    A_UNUSED(object);
    A_UNUSED(parent);

    return std::list<QWidget *>();
}

/*!
    Returns a context menu for an object property.
*/
QMenu *LegacyAssetEditor::propertyContextMenu(Object *object, const TString &property) {
    A_UNUSED(object);
    A_UNUSED(property);

    return nullptr;
}

/*!
    Returns the undo and redo stack used by this editor.
*/
UndoStack *LegacyAssetEditor::undoRedo() const {
    return m_undoRedo;
}

/*!
    Returns the current editor state for serialization.
*/
VariantMap LegacyAssetEditor::saveState() {
    return VariantMap();
}

/*!
    Restores the editor state from serialized data.
*/
void LegacyAssetEditor::restoreState(const VariantMap &data) {
    A_UNUSED(data);
}

/*!
    Returns the objects currently selected in the editor.
*/
Object::ObjectList LegacyAssetEditor::selected() const {
    return Object::ObjectList();
}
