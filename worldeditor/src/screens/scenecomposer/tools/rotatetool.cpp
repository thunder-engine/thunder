#include "rotatetool.h"

#include <QLineEdit>
#include <QDoubleValidator>

#include <cfloat>

#include <components/actor.h>
#include <components/transform.h>
#include <components/camera.h>

#include <editor/viewport/handles.h>

#include "../objectcontroller.h"

RotateTool::RotateTool(ObjectController *controller) :
        SelectTool(controller) {

    m_snap = 5.0f;
}

void RotateTool::beginControl() {
    SelectTool::beginControl();

    m_eulers.clear();
    m_quaternions.clear();
    m_positions.clear();

    for(auto it : m_controller->selectList()) {
        Transform *t = it.object->transform();
        m_eulers.push_back(t->rotation());
        m_positions.push_back(t->position());
        m_quaternions.push_back(t->quaternion());
    }
}

void RotateTool::update(bool pivot, bool local, bool snap) {
    SelectTool::update(pivot, local, snap);

    if(!m_controller->isDrag()) {
        m_position = objectPosition();
    }

    float angle = Handles::rotationTool(m_position, local ? m_quaternions.front() : Quaternion(), m_controller->isDrag());
    if(m_snap > 0) {
        angle = m_snap * int(angle / m_snap);
    }

    if(m_controller->isDrag()) {
        auto eulerIt = m_eulers.begin();
        auto posIt = m_positions.begin();
        auto quatIt = m_quaternions.begin();
        for(const auto &it : qAsConst(m_controller->selectList())) {
            Transform *tr = it.object->transform();
            Matrix4 parent;
            if(tr->parentTransform()) {
                parent = tr->parentTransform()->worldTransform();
            }
            Vector3 p((parent * *posIt) - m_position);
            Quaternion q;
            Vector3 euler(*eulerIt);

            Quaternion rot = local ? *quatIt : Quaternion();

            switch(Handles::s_Axes) {
                case Handles::AXIS_X: {
                    q = Quaternion(rot * Vector3(1.0f, 0.0f, 0.0f), angle);
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

            eulerIt++;
            quatIt++;
        }
    }
}

QLineEdit *RotateTool::snapWidget() {
    if(m_snapEditor == nullptr) {
        QDoubleValidator *validator = new QDoubleValidator(0.01f, DBL_MAX, 4);
        validator->setLocale(QLocale("C"));

        m_snapEditor = new QLineEdit();
        m_snapEditor->setValidator(validator);
        m_snapEditor->setObjectName(name());
        m_snapEditor->setText(QString::number((double)m_snap, 'f', 2));
    }

    return m_snapEditor;
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
