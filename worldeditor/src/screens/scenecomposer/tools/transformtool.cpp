#include "transformtool.h"

#include <components/actor.h>
#include <components/transform.h>
#include <components/spriterender.h>

#include <editor/viewport/handles.h>

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

        Qt::CursorShape shape = Qt::ArrowCursor;
        if(Handles::s_Axes == (Handles::TOP | Handles::BOTTOM | Handles::LEFT | Handles::RIGHT)) {
            shape = Qt::SizeAllCursor;
        } else if(Handles::s_Axes == (Handles::TOP | Handles::RIGHT)) {
            shape = Qt::SizeBDiagCursor;
        } else if(Handles::s_Axes == (Handles::TOP | Handles::LEFT)) {
            shape = Qt::SizeFDiagCursor;
        } else if(Handles::s_Axes == (Handles::BOTTOM | Handles::RIGHT)) {
            shape = Qt::SizeFDiagCursor;
        } else if(Handles::s_Axes == (Handles::BOTTOM | Handles::LEFT)) {
            shape = Qt::SizeBDiagCursor;
        } else if(Handles::s_Axes == Handles::TOP || Handles::s_Axes == Handles::BOTTOM) {
            shape = Qt::SizeVerCursor;
        } else if(Handles::s_Axes == Handles::LEFT || Handles::s_Axes == Handles::RIGHT) {
            shape = Qt::SizeHorCursor;
        }

        m_cursor = shape;
    }
}

std::string TransformTool::icon() const {
    return ":/Images/editor/Transform.png";
}

std::string TransformTool::name() const {
    return "Transform";
}

std::string TransformTool::component() const {
    return SpriteRender::metaClass()->name();
}
