#include "splinetool.h"

#include <components/actor.h>
#include <components/transform.h>
#include <components/camera.h>

#include <editor/viewport/handles.h>
#include <editor/viewport/handletools.h>

#include <gizmos.h>
#include <input.h>

#include "../../objectcontroller.h"

#include "splinepanel.h"

SplineTool::SplineTool(ObjectController *controller) :
        EditorTool(),
        m_dotColor(Vector4(0.0f, 0.0f, 0.0f, 1.0f)),
        m_dotColorSelected(Vector4(1.0f, 1.0f, 1.0f, 1.0f)),
        m_lineColor(Vector4(1.0f, 1.0f, 0.0f, 1.0f)),
        m_controller(controller),
        m_spline(nullptr),
        m_splinePanel(nullptr),
        m_dotSize(0.003f),
        m_point(-1),
        m_tangent(0),
        m_canceled(false) {

}

void SplineTool::setPoint(int point) {
    m_point = point;
    if(m_splinePanel) {
        m_splinePanel->update();
    }
}

void SplineTool::update(bool center, bool local, bool snap) {
    EditorTool::update(center, local, snap);

    Actor *actor = m_controller->selectList().begin()->object;
    if(actor) {
        Spline *spline = actor->getComponent<Spline>();
        if(spline) {
            Vector3 camera(m_controller->camera()->transform()->position());

            int hoverPoint = -1;
            int hoverTangent = 0;

            Transform *t = spline->transform();
            Matrix4 m(t->worldTransform());

            bool isDrag = m_controller->isDrag();

            const float sense = Handles::s_Sense * 0.2f;

            if(Input::isKeyDown(Input::KEY_DELETE) && m_point > -1) {
                m_controller->undoRedo()->push(new DeleteSplinePoint(this));
            }

            for(int i = 0; i < spline->pointsCount(); i++) {
                Spline::Point point(spline->point(i));

                bool selected = (i == m_point);

                Gizmos::drawBox(point.position, (camera - point.position).length() * m_dotSize, selected && (m_tangent == 0) ? m_dotColorSelected : m_dotColor, m);
                float distance = HandleTools::distanceToPoint(m, point.position, Handles::s_Mouse);
                if(distance <= sense) {
                    hoverPoint = i;
                }
                if(selected) {
                    Vector3 position(point.position);

                    Gizmos::drawLines({point.position, point.tangentIn, point.tangentOut}, {0, 1, 0, 2}, m_lineColor, m);
                    Gizmos::drawBox(point.tangentIn, (camera - point.position).length() * m_dotSize, (m_tangent == 1) ? m_dotColorSelected : m_dotColor, m);
                    Gizmos::drawBox(point.tangentOut, (camera - point.position).length() * m_dotSize, (m_tangent == 2) ? m_dotColorSelected : m_dotColor, m);

                    distance = HandleTools::distanceToPoint(m, point.tangentIn, Handles::s_Mouse);
                    if(distance <= sense) {
                        hoverTangent = 1;
                    } else {
                        distance = HandleTools::distanceToPoint(m, point.tangentOut, Handles::s_Mouse);
                        if(distance <= sense) {
                            hoverTangent = 2;
                        }
                    }

                    if(m_tangent == 1) {
                        position = point.tangentIn;
                    } else if(m_tangent == 2) {
                        position = point.tangentOut;
                    }

                    m_world = Handles::moveTool(position, Quaternion(), isDrag);
                    if(isDrag) {
                        Vector3 delta(m_world - m_savedWorld);
                        if(m_tangent == 0) {
                            point.position += delta;
                            point.tangentIn += delta;
                            point.tangentOut += delta;
                        } else {
                            if(m_tangent == 1) {
                                point.tangentIn += delta;
                                if(!point.breaked) {
                                    point.tangentOut -= delta;
                                }
                            }
                            if(m_tangent == 2) {
                                if(!point.breaked) {
                                    point.tangentIn -= delta;
                                }
                                point.tangentOut += delta;
                            }
                        }
                        m_savedWorld = m_world;
                        spline->setPoint(m_point, point);
                        if(m_splinePanel) {
                            m_splinePanel->update();
                        }
                    }
                }
            }

            if(Input::isMouseButtonUp(Input::MOUSE_LEFT) && !isDrag) {
                if(!m_canceled) {
                    if(m_point > -1 || m_tangent || hoverPoint > -1 || hoverTangent) {
                        m_controller->undoRedo()->push(new SelectSplinePoint(hoverPoint, hoverTangent, this));
                    }
                } else {
                    m_canceled = false;
                }
            }
        }
    }
}

void SplineTool::beginControl() {
    EditorTool::beginControl();

    if(m_point > -1) {
        Actor *actor = m_controller->selectList().begin()->object;
        if(actor) {
            Spline *spline = actor->getComponent<Spline>();
            if(spline) {
                Spline::Point point(spline->point(m_point));

                m_position = point.position;
                m_positionIn = point.tangentIn;
                m_positionOut = point.tangentOut;
            }
        }

        m_savedWorld = m_world;
    }
}

void SplineTool::endControl() {
    EditorTool::beginControl();

    if(m_point > -1) {
        Actor *actor = m_controller->selectList().begin()->object;
        if(actor) {
            Spline *spline = actor->getComponent<Spline>();
            if(spline) {
                Spline::Point point(spline->point(m_point));

                spline->setPoint(m_point, {m_position, m_positionIn, m_positionOut});

                m_controller->undoRedo()->push(new ChangeSplinePoint(point, this));
            }
        }
    }
}

void SplineTool::cancelControl() {
    EditorTool::beginControl();

    if(m_point > -1) {
        Actor *actor = m_controller->selectList().begin()->object;
        if(actor) {
            Spline *spline = actor->getComponent<Spline>();
            if(spline) {
                spline->setPoint(m_point, {m_position, m_positionIn, m_positionOut});
            }
        }

        m_canceled = true;
    }
}

std::string SplineTool::icon() const {
    return ":/Images/editor/Spline.png";
}

std::string SplineTool::name() const {
    return "Spline Tool";
}

std::string SplineTool::component() const {
    return Spline::metaClass()->name();
}

bool SplineTool::blockSelection() const {
    return true;
}

QWidget *SplineTool::panel() {
    if(m_splinePanel == nullptr) {
        m_splinePanel = new SplinePanel;
        m_splinePanel->setTool(this);
    }
    return m_splinePanel;
}

void SplineTool::update() {
    if(m_splinePanel) {
        m_splinePanel->update();
    }
}

Spline *SplineTool::spline() {
    Actor *actor = m_controller->selectList().begin()->object;
    if(actor) {
        return actor->getComponent<Spline>();
    }
    return nullptr;
}

SelectSplinePoint::SelectSplinePoint(int point, int tangent, SplineTool *tool, const TString &name, UndoCommand *group) :
        UndoCommand(name, group),
        m_tool(tool),
        m_point(point),
        m_tangent(tangent) {

}
void SelectSplinePoint::redo() {
    int point = m_tool->point();
    int tangent = m_tool->tangent();

    m_tool->setTangent(m_tangent);
    if(m_tangent > 0) {
        m_tool->setPoint(point);
    } else {
        m_tool->setPoint(m_point);
    }

    m_point = point;
    m_tangent = tangent;
}

ChangeSplinePoint::ChangeSplinePoint(const Spline::Point &point, SplineTool *tool, const TString &name, UndoCommand *group) :
        UndoCommand(name, group),
        m_tool(tool),
        m_point(point) {

}
void ChangeSplinePoint::redo() {
    Spline *spline = m_tool->spline();
    if(spline) {
        int index = m_tool->point();
        Spline::Point point(spline->point(index));
        spline->setPoint(index, m_point);
        m_tool->update();

        m_point = point;
    }
}

DeleteSplinePoint::DeleteSplinePoint(SplineTool *tool, const TString &name, UndoCommand *group) :
        UndoCommand(name, group),
        m_tool(tool),
        m_index(-1),
        m_tangent(0) {

}
void DeleteSplinePoint::undo() {
    Spline *spline = m_tool->spline();
    if(spline) {
        spline->insertPoint(m_index, m_point);
        m_tool->setTangent(m_tangent);
        m_tool->setPoint(m_index);
    }
}
void DeleteSplinePoint::redo() {
    Spline *spline = m_tool->spline();
    if(spline) {
        m_index = m_tool->point();
        m_tangent = m_tool->tangent();
        m_point = spline->point(m_index);
        spline->removePoint(m_index);
        m_tool->setTangent(0);
        m_tool->setPoint(-1);
    }
}

InsertSplinePoint::InsertSplinePoint(float factor, SplineTool *tool, const TString &name, UndoCommand *group) :
        UndoCommand(name, group),
        m_tool(tool),
        m_factor(factor),
        m_index(-1) {

}
void InsertSplinePoint::undo() {
    Spline *spline = m_tool->spline();
    if(spline) {
        spline->removePoint(m_index);
        m_tool->setPoint(m_index-1);
    }
}
void InsertSplinePoint::redo() {
    Spline *spline = m_tool->spline();
    if(spline) {
        m_index = m_tool->point();

        Spline::Point a = spline->point(m_index);
        if(m_index < (spline->pointsCount() - 1)) { // interpolation
            Spline::Point b = spline->point(m_index + 1);
            Vector3 v = CMIX(a.position, a.tangentOut, b.tangentIn, b.position, m_factor);
            if(a.breaked) {
                a.tangentIn = MIX(a.position, v, 0.5f);
                a.tangentOut = MIX(b.position, v, 0.5f);
            } else {
                Vector3 delta(v - a.position);
                a.tangentIn += delta;
                a.tangentOut += delta;
            }
            a.position = v;
        } else if(spline->pointsCount() > 1) { // extrapolation
            Spline::Point b = spline->point(m_index - 1);
            Vector3 delta((a.position - b.position) * m_factor);
            a.position += delta;
            a.tangentIn += delta;
            a.tangentOut += delta;
        }

        spline->insertPoint(m_index + 1, a);
        m_tool->setPoint(m_index + 1);
        m_index += 1;

    }
}
