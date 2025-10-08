#include "movetool.h"

#include <QLineEdit>
#include <QDoubleValidator>

#include <cfloat>

#include <components/actor.h>
#include <components/transform.h>

#include <editor/viewport/handles.h>

#include "../objectcontroller.h"

MoveTool::MoveTool(ObjectController *controller) :
    SelectTool(controller) {

    m_snap = 0.25f;
}

void MoveTool::beginControl() {
    SelectTool::beginControl();

    m_positions.clear();

    for(auto &it : m_controller->selectList()) {
        Transform *t = it.object->transform();
        m_positions.push_back(t->position());
    }
}

void MoveTool::update(bool center, bool local, bool snap) {
    SelectTool::update(center, local, snap);

    bool isDrag = m_controller->isDrag();

    SelectTool::SelectList &list = m_controller->selectList();
    if(!list.isEmpty()) {
        Transform *t = list.back().object->transform();

        m_world = Handles::moveTool(objectPosition(), local ? t->worldQuaternion() : Quaternion(), isDrag);
        if(isDrag) {
            Vector3 delta(m_world - m_savedWorld);
            if(snap && m_snap > 0.0f) {
                for(int32_t i = 0; i < 3; i++) {
                    delta[i] = m_snap * int(delta[i] / m_snap);
                }
            }

            auto posIt = m_positions.begin();
            for(const auto &it : qAsConst(list)) {
                Vector3 dt(local ? t->worldQuaternion() * delta : delta);
                Actor *a = dynamic_cast<Actor *>(it.object->parent());
                if(!local && a && a->transform()) {
                    dt = a->transform()->worldTransform().rotation().inverse() * delta;
                }
                if(it.object->transform()) {
                    it.object->transform()->setPosition(*posIt + dt);
                }
                posIt++;
            }
        }
    }
}

QLineEdit *MoveTool::snapWidget() {
    if(m_snapEditor == nullptr) {
        QDoubleValidator *validator = new QDoubleValidator(0.01f, DBL_MAX, 4);
        validator->setLocale(QLocale("C"));

        m_snapEditor = new QLineEdit();
        m_snapEditor->setValidator(validator);
        m_snapEditor->setObjectName(name().c_str());
        m_snapEditor->setText(QString::number((double)m_snap, 'f', 2));
    }

    return m_snapEditor;
}

std::string MoveTool::icon() const {
    return ":/Images/editor/Move.png";
}

std::string MoveTool::name() const {
    return "Move";
}

std::string MoveTool::toolTip() const {
    return QObject::tr("Select and Translate objects").toStdString();
}

std::string MoveTool::shortcut() const {
    return "Shift+T";
}
