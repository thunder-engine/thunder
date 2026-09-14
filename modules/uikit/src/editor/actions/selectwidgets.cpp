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
#include "selectwidgets.h"

SelectWidgets::SelectWidgets(const std::list<uint32_t> &objects, WidgetController *ctrl, const TString &name, UndoCommand *group) :
        UndoCommand(name, group),
        m_objects(objects),
        m_controller(ctrl) {

}

void SelectWidgets::undo() {
    SelectWidgets::redo();
}

void SelectWidgets::redo() {
    Object::ObjectList objects = m_controller->selected();

    m_controller->clear(false);
    m_controller->selectActors(m_objects);

    m_objects.clear();
    for(auto &it : objects) {
        m_objects.push_back(it->uuid());
    }
}
