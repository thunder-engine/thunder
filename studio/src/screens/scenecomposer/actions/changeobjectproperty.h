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
#ifndef CHANGEOBJECTPROPERTY_H
#define CHANGEOBJECTPROPERTY_H

#include "../objectcontroller.h"

class ChangeObjectProperty : public UndoCommand {
public:
    ChangeObjectProperty(const Object::ObjectList &objects, const TString &property, const Variant &value, ObjectController *ctrl, const TString &name, UndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    TString m_property;

    Variant m_value;

    std::list<uint32_t> m_objects;

    ObjectController *m_controller;

    uint32_t m_prefab;

};

#endif // CHANGEOBJECTPROPERTY_H
