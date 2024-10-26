#include "rotatetool.h"

#include <components/actor.h>
#include <components/transform.h>
#include <components/camera.h>

#include <editor/viewport/handles.h>

#include "../objectcontroller.h"

RotateTool::RotateTool(ObjectController *controller) :
    SelectTool(controller) {

    m_snap = 5.0f;
}

void RotateTool::update(bool pivot, bool local, bool snap) {
    SelectTool::update(pivot, local, snap);

    if(!m_controller->isDrag()) {
        m_position = objectPosition();
    }

    SelectTool::Select &sel = m_controller->selectList().back();

    float angle = Handles::rotationTool(m_position, local ? sel.quat : Quaternion(), m_controller->isDrag());
    if(m_snap > 0) {
        angle = m_snap * int(angle / m_snap);
    }

    if(m_controller->isDrag()) {
        QSet<Scene *> scenes;

        for(const auto &it : qAsConst(m_controller->selectList())) {
            Transform *tr = it.object->transform();
            Matrix4 parent;
            if(tr->parentTransform()) {
                parent = tr->parentTransform()->worldTransform();
            }
            Vector3 p((parent * it.position) - m_position);
            Quaternion q;
            Vector3 euler(it.euler);

            Quaternion rot = local ? it.quat : Quaternion();

            switch(Handles::s_Axes) {
                case Handles::AXIS_X: {
                    q = Quaternion(rot * Vector3(1.0f, 0.0f, 0.0f), angle);
                    //euler += Vector3(angle, 0.0f, 0.0f);
                    euler += rot * Vector3(angle, 0.0f, 0.0f);
                } break;
                case Handles::AXIS_Y: {
                    q = Quaternion(rot * Vector3(0.0f, 1.0f, 0.0f), angle);
                    //euler += Vector3(0.0f, angle, 0.0f);
                    euler += rot * Vector3(0.0f, angle, 0.0f);
                } break;
                case Handles::AXIS_Z: {
                    q = Quaternion(rot * Vector3(0.0f, 0.0f, 1.0f), angle);
                    euler += rot * Vector3(0.0f, 0.0f, angle);
                } break;
                default: {
                    Vector3 axis(m_controller->camera()->transform()->quaternion() * Vector3(0.0f, 0.0f, 1.0f));
                    axis.normalize();
                    q = Quaternion(axis, angle);
                    euler = q.euler();
                } break;
            }
            tr->setPosition(parent.inverse() * (m_position + q * p));
            tr->setRotation(euler);
            //tr->setQuaternion(q);

            scenes.insert(it.object->scene());
        }
    }
}

QString RotateTool::icon() const {
    return ":/Images/editor/Rotate.png";
}

QString RotateTool::name() const {
    return "Rotate";
}

QString RotateTool::toolTip() const {
    return QObject::tr("Select and Rotate objects");
}

QString RotateTool::shortcut() const {
    return "Shift+R";
}
