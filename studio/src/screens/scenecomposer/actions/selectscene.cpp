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
#include "selectscene.h"

#include "components/scene.h"
#include "components/world.h"

SelectScene::SelectScene(Scene *scene, ObjectController *ctrl, const TString &name, UndoCommand *group) :
        UndoCommand(name, group),
        m_controller(ctrl),
        m_object(scene->uuid()) {

}

void SelectScene::undo() {
    SelectScene::redo();
}

void SelectScene::redo() {
    uint32_t back = m_controller->scene()->uuid();

    Object *object = Engine::findObject(m_object);
    if(object && dynamic_cast<Scene *>(object)) {
        Engine::world()->setActiveScene(static_cast<Scene *>(object));
        m_object = back;
    }
}
