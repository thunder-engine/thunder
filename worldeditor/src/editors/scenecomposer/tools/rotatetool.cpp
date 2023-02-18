#include "rotatetool.h"

#include <components/actor.h>
#include <components/transform.h>
#include <components/camera.h>

#include <editor/viewport/handles.h>

#include "../objectctrl.h"

RotateTool::RotateTool(ObjectCtrl *controller, SelectList &selection) :
    SelectTool(controller, selection) {

}

void RotateTool::update(bool pivot, bool local, float snap) {
    SelectTool::update(pivot, local, snap);

    if(!m_controller->isDrag()) {
        m_position = objectPosition();
    }

    Transform *t = m_Selected.back().object->transform();

    float angle = Handles::rotationTool(m_position, local ? t->worldQuaternion() : Quaternion(), m_controller->isDrag());
    if(snap > 0) {
        angle = snap * int(angle / snap);
    }

    if(m_controller->isDrag()) {
        QSet<Scene *> scenes;

        for(const auto &it : qAsConst(m_Selected)) {
            Transform *tr = it.object->transform();
            Matrix4 parent;
            if(tr->parentTransform()) {
                parent = tr->parentTransform()->worldTransform();
            }
            Vector3 p((parent * it.position) - m_position);
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
                    Vector3 axis(m_controller->camera()->transform()->quaternion() * Vector3(0.0f, 0.0f, 1.0f));
                    axis.normalize();
                    q = Quaternion(axis, angle);
                    euler = q.euler();
                } break;
            }
            tr->setPosition(parent.inverse() * (m_position + q * p));
            tr->setRotation(euler);

            scenes.insert(it.object->scene());
        }
        for(auto it : scenes) {
            emit m_controller->objectsUpdated(it);
        }
        emit m_controller->objectsChanged(m_controller->selected(), "Rotation");
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
