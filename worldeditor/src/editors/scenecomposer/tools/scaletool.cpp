#include "scaletool.h"

#include <components/actor.h>
#include <components/transform.h>
#include <components/camera.h>

#include <editor/viewport/handles.h>

#include "../objectctrl.h"

ScaleTool::ScaleTool(ObjectCtrl *controller, SelectList &selection) :
    SelectTool(controller, selection) {

}

void ScaleTool::update(bool pivot, bool local, float snap) {
    SelectTool::update(pivot, local, snap);

    bool isDrag = m_controller->isDrag();

    if(!isDrag) {
        m_position = objectPosition();
        Handles::s_Axes = 0;
    }

    Transform *t = m_selected.back().object->transform();

    m_world = Handles::scaleTool(m_position, local ? t->worldQuaternion() : Quaternion(), isDrag);

    Camera *camera = Camera::current();
    if(isDrag && camera) {
        Vector3 normal = m_position - camera->transform()->position();

        Vector3 delta(m_world - m_savedWorld);

        Vector3 s;
        if(Handles::s_Axes & Handles::AXIS_X) {
            float scale = (normal.x < 0) ? delta.x : -delta.x;
            if(snap > 0) {
                scale = snap * int(scale / snap);
            }
            s.x += scale;
        }
        if(Handles::s_Axes & Handles::AXIS_Y) {
            float scale = (normal.y < 0) ? delta.y : -delta.y;
            if(snap > 0) {
                scale = snap * int(scale / snap);
            }
            s.y += scale;
        }
        if(Handles::s_Axes & Handles::AXIS_Z) {
            float scale = (normal.z < 0) ? delta.z : -delta.z;
            if(snap > 0) {
                scale = snap * int(scale / snap);
            }
            s.z += scale;
        }

        QSet<Scene *> scenes;
        for(const auto &it : qAsConst(m_selected)) {
            Transform *tr = it.object->transform();
            Matrix4 parent;
            if(tr->parentTransform()) {
                parent = tr->parentTransform()->worldTransform();
            }

            scenes.insert(it.object->scene());

            Vector3 v(it.scale + s);
            tr->setScale(v);

            Vector3 p(parent * it.position - m_position);
            tr->setPosition(parent.inverse() * (v * p + m_position));
        }
        for(auto it : scenes) {
            emit m_controller->objectsUpdated(it);
        }
        emit m_controller->objectsChanged(m_controller->selected(), "Scale");
    }
}

QString ScaleTool::icon() const {
    return ":/Images/editor/Scale.png";
}

QString ScaleTool::name() const {
    return "scaleTool";
}

QString ScaleTool::toolTip() const {
    return QObject::tr("Select and Scale objects");
}

QString ScaleTool::shortcut() const {
    return "Shift+S";
}
