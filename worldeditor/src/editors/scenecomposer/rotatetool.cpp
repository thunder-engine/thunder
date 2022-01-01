#include "rotatetool.h"

#include <components/actor.h>
#include <components/transform.h>
#include <components/camera.h>

#include <editor/viewport/handles.h>

#include "objectctrl.h"

RotateTool::RotateTool(ObjectCtrl *controller, SelectList &selection) :
    SelectTool(controller, selection) {

}

void RotateTool::update(bool pivot, bool local, float snap) {
    A_UNUSED(pivot);
    A_UNUSED(local);

    if(!m_pController->isDrag()) {
        m_Position = objectPosition();
    }

    Transform *t = m_Selected.back().object->transform();

    float angle = Handles::rotationTool(m_Position, local ? t->worldQuaternion() : Quaternion(), m_pController->isDrag());
    if(snap > 0) {
        angle = snap * int(angle / snap);
    }

    if(m_pController->isDrag()) {
        for(const auto &it : m_Selected) {
            Transform *tr = it.object->transform();
            Matrix4 parent;
            if(tr->parentTransform()) {
                parent = tr->parentTransform()->worldTransform();
            }
            Vector3 p((parent * it.position) - m_Position);
            Quaternion q;
            Vector3 euler(it.euler);
            switch(Handles::s_Axes) {
                case Handles::AXIS_X: {
                    q = Quaternion(Vector3(1.0f, 0.0f, 0.0f), angle);
                    euler += Vector3(angle, 0.0f, 0.0f);
                } break;
                case Handles::AXIS_Y: {
                    q = Quaternion(Vector3(0.0f, 1.0f, 0.0f), angle);
                    euler += Vector3(0.0f, angle, 0.0f);
                } break;
                case Handles::AXIS_Z: {
                    q = Quaternion(Vector3(0.0f, 0.0f, 1.0f), angle);
                    euler += Vector3(0.0f, 0.0f, angle);
                } break;
                default: {
                    Vector3 axis(m_pController->camera()->actor()->transform()->quaternion() * Vector3(0.0f, 0.0f, 1.0f));
                    axis.normalize();
                    q = Quaternion(axis, angle);
                    euler = q.euler();
                } break;
            }
            tr->setPosition(parent.inverse() * (m_Position + q * p));
            tr->setRotation(euler);
        }
        m_pController->objectsUpdated();
        m_pController->objectsChanged(m_pController->selected(), "Rotation");
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
