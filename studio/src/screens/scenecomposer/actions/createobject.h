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
#ifndef CREATEOBJECT_H
#define CREATEOBJECT_H

#include "../objectcontroller.h"

class CreateObject : public UndoCommand {
public:
    CreateObject(const TString &type, Object *parent, const Vector3 &position, ObjectController *ctrl, UndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    Object *createObject(Object *parent);

    void resolveUUID(Object::ObjectList &list, Object *root, bool generate);

protected:
    std::list<uint32_t> m_objects;
    std::list<uint32_t> m_selected;
    std::unordered_map<uint32_t, uint32_t> m_staticIds;

    TString m_type;

    Vector3 m_position;

    ObjectController *m_controller;

    uint32_t m_parent;

    uint32_t m_prefab;

};

#endif // CREATEOBJECT_H
