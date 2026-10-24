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
#include "movetool.h"

#include <components/actor.h>
#include <components/transform.h>

#include <viewport/handles.h>

#include "../objectcontroller.h"

MoveTool::MoveTool(ObjectController *controller) :
    SelectTool(controller) {

    m_snap = 0.25f;
}

void MoveTool::beginControl() {
    SelectTool::beginControl();

    m_positions.clear();

    for(auto &it : m_controller->selectList()) {
        Transform *t = it.object->transform();
        m_positions.push_back(t->position());
    }
}

void MoveTool::update(bool center, bool local, bool snap) {
    SelectTool::update(center, local, snap);

    bool isDrag = m_controller->isDrag();

    SelectTool::SelectList &list = m_controller->selectList();
    if(!list.empty()) {
        Transform *t = list.back().object->transform();

        m_world = Handles::moveTool(objectPosition(), local ? t->worldQuaternion() : Quaternion(), isDrag);
        if(isDrag) {
            Vector3 delta(m_world - m_savedWorld);
            if(snap && m_snap > 0.0f) {
                for(int32_t i = 0; i < 3; i++) {
                    delta[i] = m_snap * int(delta[i] / m_snap);
                }
            }

            auto posIt = m_positions.begin();
            for(const auto &it : list) {
                Vector3 dt(local ? t->worldQuaternion() * delta : delta);
                Actor *a = dynamic_cast<Actor *>(it.object->parent());
                if(!local && a && a->transform()) {
                    dt = a->transform()->worldTransform().rotation().inverse() * delta;
                }
                if(it.object->transform()) {
                    it.object->transform()->setPosition(*posIt + dt);
                }
                posIt++;
            }
        }
    }
}

TString MoveTool::icon() const {
    return ":/Images/editor/Move.png";
}

TString MoveTool::name() const {
    return "Move";
}

TString MoveTool::toolTip() const {
    return "Select and Move objects";
}

TString MoveTool::shortcut() const {
    return "Shift+T";
}
