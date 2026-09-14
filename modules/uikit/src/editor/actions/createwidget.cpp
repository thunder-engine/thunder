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
#include "createwidget.h"

#include <actor.h>
#include "components/widget.h"

CreateWidget::CreateWidget(const TString &type, Scene *scene, WidgetController *ctrl, UndoCommand *group) :
        UndoCommand(TString("Create ") + type, group),
        m_type(type),
        m_controller(ctrl) {

}

void CreateWidget::undo() {
    for(auto uuid : m_objects) {
        Object *object = Engine::findObject(uuid);
        if(object) {
            delete object;
        }
    }

    m_controller->clear(false);
    m_controller->selectActors(m_selected);

    emit m_controller->sceneUpdated();
}

void CreateWidget::redo() {
    m_objects.clear();
    m_selected.clear();

    for(auto it : m_controller->selected()) {
        m_selected.push_back(it->uuid());
    }

    Widget *root = m_controller->root();
    Object *parent = root->actor();
    if(!m_controller->selected().empty()) {
        parent = m_controller->selected().front();        
    }

    Object *object = Engine::composeActor(m_type, m_type, parent);
    if(object) {
        m_objects.push_back(object->uuid());
    }

    m_controller->clear(false);
    m_controller->selectActors(m_objects);

    emit m_controller->sceneUpdated();
}
