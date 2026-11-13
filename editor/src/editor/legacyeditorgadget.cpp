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
#include "editor/legacyeditorgadget.h"

#include "editor/legacyasseteditor.h"

#include <algorithm>
#include <set>

/*!
    \class LegacyEditorGadget
    \brief Provides the base interface for dockable editor gadgets.
    \inmodule Editor

    LegacyEditorGadget receives editor updates and selection changes and can publish
    object selection and property change notifications.

    \fn void LegacyEditorGadget::setCurrentEditor(LegacyAssetEditor *editor)

    Assigns the current asset editor to the gadget.

    \fn void LegacyEditorGadget::updated()

    Emitted when the gadget's displayed data has been updated.

    \fn void LegacyEditorGadget::objectsSelected(const Object::ObjectList &objects, bool force)

    Emitted when the gadget selects \a objects. If \a force is true, the selection
    is applied even when it matches the current selection.

    \fn void LegacyEditorGadget::objectsChanged(const Object::ObjectList &objects, const TString &property, Variant value)

    Emitted when \a property changes it's \a value on the supplied \a objects.

    \fn void LegacyEditorGadget::onUpdated()

    Handles an update notification from the editor.

    \fn void LegacyEditorGadget::onSelectionChanged()

    Handles a change to the editor selection.

    \fn void LegacyEditorGadget::onObjectsChanged(const Object::ObjectList &objects, const TString &property, Variant value)

    Handles changes of \a value to \a property on the supplied \a objects.
*/

/*!
    Constructs an editor gadget with the specified Qt \a parent.
*/
LegacyEditorGadget::LegacyEditorGadget(QWidget *parent) :
        QWidget(parent),
        m_currentEditor(nullptr) {

}

LegacyAssetEditor *LegacyEditorGadget::currentEditor() {
    return m_currentEditor;
}

void LegacyEditorGadget::setCurrentEditor(LegacyAssetEditor *editor) {
    if(m_currentEditor) {
        QObject::disconnect(this, &LegacyEditorGadget::updated, m_currentEditor, &LegacyAssetEditor::onUpdated);
        QObject::disconnect(this, &LegacyEditorGadget::objectsSelected, m_currentEditor, &LegacyAssetEditor::onObjectsSelected);

        QObject::disconnect(m_currentEditor, &LegacyAssetEditor::objectsChanged, this, &LegacyEditorGadget::onObjectsChanged);
    }

    QObject::connect(this, &LegacyEditorGadget::updated, editor, &LegacyAssetEditor::onUpdated);
    QObject::connect(this, &LegacyEditorGadget::objectsSelected, editor, &LegacyAssetEditor::onObjectsSelected);

    QObject::connect(editor, &LegacyAssetEditor::objectsChanged, this, &LegacyEditorGadget::onObjectsChanged);

    m_currentEditor = editor;
}

Object::ObjectList LegacyEditorGadget::selected() {
    return Object::ObjectList();
}

TString LegacyEditorGadget::propertyWithObjectTypes(const Object::ObjectList &objects, const TString &property) {
    std::set<TString> types;
    for(Object *object : objects) {
        if(object && object->metaObject()) {
            types.insert(object->metaObject()->name());
        }
    }

    if(types.empty()) {
        return property;
    }

    StringList names(types.begin(), types.end());
    return TString::join(names, "|") + "::" + property;
}

Object::ObjectList LegacyEditorGadget::objectsForProperty(const Object::ObjectList &objects, const TString &property, TString &name) {
    name = property;

    int separator = property.indexOf("::");
    if(separator < 0) {
        return objects;
    }

    StringList types = property.left(separator).split('|');
    if(types.empty()) {
        return objects;
    }

    name = property.mid(separator + 2, property.length() - separator - 2);

    Object::ObjectList result;
    for(Object *object : objects) {
        if(object && object->metaObject()) {
            TString type = object->metaObject()->name();
            if(std::find(types.begin(), types.end(), type) != types.end()) {
                result.push_back(object);
            }
        }
    }

    return result;
}
