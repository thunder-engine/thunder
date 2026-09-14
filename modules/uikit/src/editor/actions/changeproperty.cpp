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
#include "changeproperty.h"

ChangeProperty::ChangeProperty(const Object::ObjectList &objects, const TString &property, const Variant &value, WidgetController *ctrl, const TString &name, UndoCommand *group) :
        UndoCommand(name, group),
        m_value(value),
        m_property(property),
        m_controller(ctrl) {

    for(auto it : objects) {
        m_objects.push_back(it->uuid());
    }
}

void ChangeProperty::undo() {
    ChangeProperty::redo();
}

void ChangeProperty::redo() {
    std::list<Object *> objects;

    Variant value(m_value);

    for(auto it : m_objects) {
        Object *object = Engine::findObject(it);
        if(object) {
            m_value = object->property(m_property.data());
            object->setProperty(m_property.data(), value);

            objects.push_back(object);
        }
    }

    if(!objects.empty()) {
        emit m_controller->propertyChanged(objects, m_property, value);
    }

    emit m_controller->sceneUpdated();
}
