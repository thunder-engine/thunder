#include "movetool.h"

#include <components/actor.h>
#include <components/transform.h>

#include <editor/viewport/handles.h>

#include "../objectcontroller.h"

MoveTool::MoveTool(ObjectController *controller, SelectList &selection) :
    SelectTool(controller, selection) {

    m_snap = 0.25f;

}

void MoveTool::update(bool center, bool local, bool snap) {
    SelectTool::update(center, local, snap);

    bool isDrag = m_controller->isDrag();

    Transform *t = m_selected.back().object->transform();

    m_world = Handles::moveTool(objectPosition(), local ? t->worldQuaternion() : Quaternion(), isDrag);
    if(isDrag) {
        Vector3 delta(m_world - m_savedWorld);
        if(snap && m_snap > 0.0f) {
            for(int32_t i = 0; i < 3; i++) {
                delta[i] = m_snap * int(delta[i] / m_snap);
            }
        }
        QSet<Scene *> scenes;
        for(const auto &it : qAsConst(m_selected)) {
            Vector3 dt(local ? t->worldQuaternion() * delta : delta);
            Actor *a = dynamic_cast<Actor *>(it.object->parent());
            if(!local && a && a->transform()) {
                dt = a->transform()->worldTransform().rotation().inverse() * delta;
            }
            if(it.object->transform()) {
                it.object->transform()->setPosition(it.position + dt);
            }
            scenes.insert(it.object->scene());
        }
    }
}

QString MoveTool::icon() const {
    return ":/Images/editor/Move.png";
}

QString MoveTool::name() const {
    return "Move";
}

QString MoveTool::toolTip() const {
    return QObject::tr("Select and Translate objects");
}

QString MoveTool::shortcut() const {
    return "Shift+T";
}
