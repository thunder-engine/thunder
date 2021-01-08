#include "scaletool.h"

#include <components/actor.h>
#include <components/transform.h>

#include <editor/handles.h>

#include "objectctrl.h"
#include "undomanager.h"

ScaleTool::ScaleTool(ObjectCtrl *controller, SelectMap &selection) :
    SelectTool(controller, selection),
    m_ScaleGrid(0.0f) {

}

void ScaleTool::update() {
    bool isDrag = m_pController->isDrag();
    if(!isDrag) {
        Handles::s_Axes = Handles::AXIS_X | Handles::AXIS_Y | Handles::AXIS_Z;
    }

    m_World = Handles::scaleTool(objectPosition(), Quaternion(), isDrag);

    if(isDrag) {
        Vector3 delta(m_World - m_SavedWorld);
        float scale = (delta.x + delta.y + delta.z) * 0.01f;
        if(m_ScaleGrid > 0) {
            scale = m_ScaleGrid * int(scale / m_ScaleGrid);
        }

        Vector3 s;
        if(Handles::s_Axes & Handles::AXIS_X) {
            s += Vector3(scale, 0, 0);
        }
        if(Handles::s_Axes & Handles::AXIS_Y) {
            s += Vector3(0, scale, 0);
        }
        if(Handles::s_Axes & Handles::AXIS_Z) {
            s += Vector3(0, 0, scale);
        }

        for(const auto &it : m_Selected) {
            Transform *tr = it.object->transform();
            Matrix4 parent;
            if(tr->parentTransform()) {
                parent = tr->parentTransform()->worldTransform();
            }
            Vector3 p((parent * it.position) - m_Position);
            Vector3 v(it.scale + s);
            tr->setPosition(parent.inverse() * (m_Position + v * p));
            tr->setScale(v);
        }
        m_pController->objectsUpdated();
        m_pController->objectsChanged(m_pController->selected(), "Scale");
    }
}

void ScaleTool::endControl() {
    VariantList pos;
    VariantList scl;
    Object::ObjectList objects;
    for(auto it : m_Selected) {
        Transform *t = it.object->transform();
        pos.push_back(t->position());
        scl.push_back(t->scale());
        objects.push_back(t);
        t->setPosition(it.position);
        t->setScale(it.scale);
    }
    QUndoCommand *group = new QUndoCommand("Scale");
    new PropertyObjects(objects, "position", pos, m_pController, "", group);
    new PropertyObjects(objects, "scale", scl, m_pController, "", group);
    UndoManager::instance()->push(group);
}

QString ScaleTool::icon() const {
    return ":/Images/editor/Scale.png";
}

QString ScaleTool::name() const {
    return "scaleTool";
}
