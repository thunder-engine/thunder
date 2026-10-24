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
#ifndef SELECTOBJECTS_H
#define SELECTOBJECTS_H

#include "../objectcontroller.h"

class SelectObjects : public UndoCommand {
public:
    SelectObjects(const std::list<uint32_t> &objects, ObjectController *ctrl, const TString &name = "Selection Change", UndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    std::list<uint32_t> m_objects;

    ObjectController *m_controller;

};

#endif // SELECTOBJECTS_H
