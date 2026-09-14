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
#ifndef PARENTOBJECTS_H
#define PARENTOBJECTS_H

#include "../objectcontroller.h"

class ParentObjects : public UndoCommand {
public:
    ParentObjects(const Object::ObjectList &objects, Object *parent, int32_t position, ObjectController *ctrl, const TString &name = "Parent Change", UndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    std::unordered_map<uint32_t, uint32_t> m_parentCache;
    std::list<uint32_t> m_objects;

    ObjectController *m_controller;

    uint32_t m_prefab;

    uint32_t m_parent;

    int32_t m_position;

};

#endif // PARENTOBJECTS_H
