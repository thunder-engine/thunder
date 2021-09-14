#include "scaletool.h"

#include <components/actor.h>
#include <components/transform.h>
#include <components/camera.h>

#include <editor/handles.h>

#include "objectctrl.h"
#include "undomanager.h"

ScaleTool::ScaleTool(ObjectCtrl *controller, SelectList &selection) :
    SelectTool(controller, selection) {

}

void ScaleTool::update(bool pivot, bool local, float snap) {
    A_UNUSED(pivot);
    bool isDrag = m_pController->isDrag();

    if(!isDrag) {
        m_Position = objectPosition();
        Handles::s_Axes = 0;
    }

    Transform *t = m_Selected.back().object->transform();

    m_World = Handles::scaleTool(m_Position, local ? t->worldQuaternion() : Quaternion(), isDrag);

    Camera *camera = Camera::current();
    if(isDrag && camera) {
        Vector3 normal = m_Position - camera->actor()->transform()->position();

        Vector3 delta(m_World - m_SavedWorld);

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

        for(const auto &it : m_Selected) {
            Transform *tr = it.object->transform();
            Matrix4 parent;
            if(tr->parentTransform()) {
                parent = tr->parentTransform()->worldTransform();
            }

            Vector3 v(it.scale + s);
            tr->setScale(v);

            Vector3 p(parent * it.position - m_Position);
            tr->setPosition(parent.inverse() * (v * p + m_Position));
        }
        m_pController->objectsUpdated();
        m_pController->objectsChanged(m_pController->selected(), "Scale");
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
    return "S";
}
