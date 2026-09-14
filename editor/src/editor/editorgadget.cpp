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
#include "editor/editorgadget.h"

/*!
    \class EditorGadget
    \brief Provides the base interface for dockable editor gadgets.
    \inmodule Editor

    EditorGadget receives editor updates and selection changes and can publish
    object selection and property change notifications.

    \fn void EditorGadget::setCurrentEditor(AssetEditor *editor)

    Assigns the current asset editor to the gadget.

    \fn void EditorGadget::updated()

    Emitted when the gadget's displayed data has been updated.

    \fn void EditorGadget::objectsSelected(const Object::ObjectList &objects, bool force)

    Emitted when the gadget selects \a objects. If \a force is true, the selection
    is applied even when it matches the current selection.

    \fn void EditorGadget::objectsChanged(const Object::ObjectList &objects, const TString &property, Variant value)

    Emitted when \a property changes it's \a value on the supplied \a objects.

    \fn void EditorGadget::onUpdated()

    Handles an update notification from the editor.

    \fn void EditorGadget::onSelectionChanged()

    Handles a change to the editor selection.

    \fn void EditorGadget::onObjectsChanged(const Object::ObjectList &objects, const TString &property, Variant value)

    Handles changes of \a value to \a property on the supplied \a objects.
*/

/*!
    Constructs an editor gadget with the specified Qt \a parent.
*/
EditorGadget::EditorGadget(QWidget *parent) :
    QWidget(parent) {

}
