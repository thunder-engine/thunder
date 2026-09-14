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
#include "transformtool.h"

#include <components/actor.h>
#include <components/transform.h>
#include <components/spriterender.h>

#include <viewport/handles.h>
#include <input.h>

#include "../objectcontroller.h"

TransformTool::TransformTool(ObjectController *controller) :
    SelectTool(controller) {

}

void TransformTool::update(bool center, bool local, bool snap) {
    SelectTool::update(center, local, snap);

    SelectTool::SelectList &list = m_controller->selectList();
    if(!list.isEmpty()) {
        Actor *actor = list.back().object;
        SpriteRender *render = actor->getComponent<SpriteRender>();
        if(render) {
            bool isDrag = m_controller->isDrag();
            AABBox bb(render->bound());
            int axis = 0;

            Handles::s_Color = Handles::s_Normal;
            m_world = Handles::rectTool(bb.center, bb.extent * 2.0f, axis, true, isDrag);

            if(isDrag) {
                Transform *t = render->transform();
                Vector3 worldScale(t->worldScale());
                Vector2 delta(m_world - m_savedWorld);

                Vector2 size(render->size());
                Vector2 pivot(render->sprite()->pivot());

                Vector2 min(-size * pivot);
                Vector2 max(min + size);

                Vector3 positionOffset;

                if(Handles::s_Axes == (Handles::TOP | Handles::BOTTOM | Handles::LEFT | Handles::RIGHT)) {
                    positionOffset = Vector3(delta);
                } else {
                    if(Handles::s_Axes & Handles::TOP) {
                        max.y += delta.y / worldScale.y;
                        positionOffset.y = delta.y * pivot.y;
                    } else if(Handles::s_Axes & Handles::BOTTOM) {
                        min.y += delta.y / worldScale.y;
                        positionOffset.y = delta.y * (1.0f - pivot.y);
                    }
                    if(Handles::s_Axes & Handles::LEFT) {
                        min.x += delta.x / worldScale.x;
                        positionOffset.x = delta.x * (1.0f - pivot.x);
                    } else if(Handles::s_Axes & Handles::RIGHT) {
                        max.x += delta.x / worldScale.x;
                        positionOffset.x = delta.x * pivot.x;
                    }
                }

                render->setSize(max - min);

                t->setPosition(t->position() + positionOffset);

                m_savedWorld = m_world;
            }
        }

        Input::CursorShape shape = Input::CURSOR_ARROW;
        if(Handles::s_Axes == (Handles::TOP | Handles::BOTTOM | Handles::LEFT | Handles::RIGHT)) {
            shape = Input::CURSOR_ALLSIZE;
        } else if(Handles::s_Axes == (Handles::TOP | Handles::RIGHT)) {
            shape = Input::CURSOR_BDIAGSIZE;
        } else if(Handles::s_Axes == (Handles::TOP | Handles::LEFT)) {
            shape = Input::CURSOR_FDIAGSIZE;
        } else if(Handles::s_Axes == (Handles::BOTTOM | Handles::RIGHT)) {
            shape = Input::CURSOR_FDIAGSIZE;
        } else if(Handles::s_Axes == (Handles::BOTTOM | Handles::LEFT)) {
            shape = Input::CURSOR_BDIAGSIZE;
        } else if(Handles::s_Axes == Handles::TOP || Handles::s_Axes == Handles::BOTTOM) {
            shape = Input::CURSOR_VERSIZE;
        } else if(Handles::s_Axes == Handles::LEFT || Handles::s_Axes == Handles::RIGHT) {
            shape = Input::CURSOR_HORSIZE;
        }

        Input::mouseSetCursor(shape);
    }
}

TString TransformTool::icon() const {
    return ":/Images/editor/Transform.png";
}

TString TransformTool::name() const {
    return "Transform";
}

TString TransformTool::component() const {
    return SpriteRender::metaClass()->name();
}
