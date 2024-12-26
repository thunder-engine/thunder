#include "scaletool.h"

#include <QLineEdit>
#include <QDoubleValidator>

#include <cfloat>

#include <components/actor.h>
#include <components/transform.h>
#include <components/camera.h>

#include <editor/viewport/handles.h>

#include "../objectcontroller.h"

ScaleTool::ScaleTool(ObjectController *controller) :
    SelectTool(controller) {

    m_snap = 1.0f;
}

void ScaleTool::beginControl() {
    SelectTool::beginControl();

    m_scales.clear();
    m_positions.clear();

    for(auto it : m_controller->selectList()) {
        Transform *t = it.object->transform();
        m_scales.push_back(t->scale());
        m_positions.push_back(t->position());
    }
}

void ScaleTool::update(bool center, bool local, bool snap) {
    SelectTool::update(center, local, snap);

    bool isDrag = m_controller->isDrag();

    if(!isDrag) {
        m_position = objectPosition();
        Handles::s_Axes = 0;
    }

    Transform *t = m_controller->selectList().back().object->transform();

    m_world = Handles::scaleTool(m_position, local ? t->worldQuaternion() : Quaternion(), isDrag);

    Camera *camera = Camera::current();
    if(isDrag && camera) {
        Vector3 normal = m_position - camera->transform()->position();

        Vector3 delta(m_world - m_savedWorld);

        Vector3 s;
        if(Handles::s_Axes & Handles::AXIS_X) {
            float scale = (normal.x < 0) ? delta.x : -delta.x;
            if(m_snap > 0) {
                scale = m_snap * int(scale / m_snap);
            }
            s.x += scale;
        }
        if(Handles::s_Axes & Handles::AXIS_Y) {
            float scale = (normal.y < 0) ? delta.y : -delta.y;
            if(m_snap > 0) {
                scale = m_snap * int(scale / m_snap);
            }
            s.y += scale;
        }
        if(Handles::s_Axes & Handles::AXIS_Z) {
            float scale = (normal.z < 0) ? delta.z : -delta.z;
            if(m_snap > 0) {
                scale = m_snap * int(scale / m_snap);
            }
            s.z += scale;
        }

        auto scaleIt = m_scales.begin();
        auto positionIt = m_positions.begin();
        for(const auto &it : qAsConst(m_controller->selectList())) {
            Transform *tr = it.object->transform();
            Matrix4 parent;
            if(tr->parentTransform()) {
                parent = tr->parentTransform()->worldTransform();
            }

            Vector3 v(*scaleIt + s);
            tr->setScale(v);

            Vector3 p(parent * *positionIt - m_position);
            tr->setPosition(parent.inverse() * (v * p + m_position));

            scaleIt++;
            positionIt++;
        }
    }
}

QLineEdit *ScaleTool::snapWidget() {
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

QString ScaleTool::icon() const {
    return ":/Images/editor/Scale.png";
}

QString ScaleTool::name() const {
    return "Scale";
}

QString ScaleTool::toolTip() const {
    return QObject::tr("Select and Scale objects");
}

QString ScaleTool::shortcut() const {
    return "Shift+S";
}
