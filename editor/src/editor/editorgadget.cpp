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

EditorGadget::EditorGadget() :
        Widget() {
}

void EditorGadget::updated() {
    emitSignal(_SIGNAL(updated()));
}

void EditorGadget::objectsSelected(const Object::ObjectList &objects, bool force) {
    //emitSignal(_SIGNAL(objectsSelected(Object::ObjectList,bool)), Variant(objects), Variant(force));
}

void EditorGadget::objectsChanged(const Object::ObjectList &objects, const TString &property, const Variant &value) {
    //emitSignal(_SIGNAL(objectsChanged(Object::ObjectList,TString,Variant)),
    //           Variant(objects), Variant(property), value);
}

void EditorGadget::onUpdated() {

}

void EditorGadget::onSelectionChanged() {

}

void EditorGadget::onObjectsChanged(const Object::ObjectList &objects, const TString &property, const Variant &value) {

}
