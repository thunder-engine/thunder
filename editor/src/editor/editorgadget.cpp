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

#include "editor/asseteditor.h"

#include <algorithm>
#include <set>

EditorGadget::EditorGadget() :
        Widget(),
        m_currentEditor(nullptr) {
}

AssetEditor *EditorGadget::currentEditor() const {
    return m_currentEditor;
}

void EditorGadget::setCurrentEditor(AssetEditor *editor) {
    if(m_currentEditor) {
        disconnect(this, _SIGNAL(updated()), m_currentEditor, _SLOT(onUpdated()));
        disconnect(this, _SIGNAL(objectsSelected(bool)), m_currentEditor, _SLOT(onObjectsSelected(bool)));
        disconnect(m_currentEditor, _SIGNAL(objectsChanged(TString,Variant)), this, _SLOT(onObjectsChanged(TString,Variant)));
    }

    m_currentEditor = editor;
    if(m_currentEditor) {
        connect(m_currentEditor, _SIGNAL(objectsChanged(TString,Variant)), this, _SLOT(onObjectsChanged(TString,Variant)));
        connect(this, _SIGNAL(updated()), m_currentEditor, _SLOT(onUpdated()));
        connect(this, _SIGNAL(objectsSelected(bool)), m_currentEditor, _SLOT(onObjectsSelected(bool)));
    }
}

Object::ObjectList EditorGadget::selected() {
    return Object::ObjectList();
}

TString EditorGadget::propertyWithObjectTypes(const Object::ObjectList &objects, const TString &property) {
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

Object::ObjectList EditorGadget::objectsForProperty(const Object::ObjectList &objects, const TString &property, TString &name) {
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

void EditorGadget::updated() {
    emitSignal(_SIGNAL(updated()));
}

void EditorGadget::objectsSelected(bool force) {
    emitSignal(_SIGNAL(objectsSelected(bool)), force);
}

void EditorGadget::objectsChanged(const TString &property, Variant value) {
    emitSignal(_SIGNAL(objectsChanged(TString,Variant)), property, value);
}

void EditorGadget::onUpdated() {

}

void EditorGadget::onSelectionChanged() {

}

void EditorGadget::onObjectsChanged(const TString &property, Variant value) {
    A_UNUSED(property);
    A_UNUSED(value);
}
