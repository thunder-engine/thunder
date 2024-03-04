#include "widgettool.h"

#include <gizmos.h>

#include <actor.h>
#include <transform.h>
#include <camera.h>
#include <renderable.h>
#include <renderable.h>
#include <editor/viewport/handles.h>

#include "components/recttransform.h"
#include "../widgetcontroller.h"

const Vector3 cornerA(20.0f,-10.0f, 0.0f);
const Vector3 cornerB(10.0f,-20.0f, 0.0f);
const Vector3 cornerC(20.0f, 10.0f, 0.0f);
const Vector3 cornerD(10.0f, 20.0f, 0.0f);

WidgetTool::WidgetTool(WidgetController *controller, SelectList &selection) :
        EditorTool(selection),
        m_controller(controller) {

}

void WidgetTool::beginControl() {
    EditorTool::beginControl();

    m_savedWorld = m_world;
}

void WidgetTool::update(bool pivot, bool local, bool snap) {
    EditorTool::update(pivot, local, snap);

    RectTransform *rect = static_cast<RectTransform *>(m_selected.front().object->transform());

    RectTransform *parent = dynamic_cast<RectTransform *>(rect->parentTransform());
    if(parent) {
        AABBox bb(parent->bound());

        Vector2 minAnchor = rect->minAnchors();
        Vector2 maxAnchor = rect->maxAnchors();

        Vector3 lt(bb.center.x - ((0.5f - minAnchor.x) * bb.extent.x * 2.0f),
                   bb.center.y + ((maxAnchor.y - 0.5f) * bb.extent.y * 2.0f), 0.0f);

        Vector3 lb(bb.center.x - ((0.5f - minAnchor.x) * bb.extent.x * 2.0f),
                   bb.center.y - ((0.5f - minAnchor.y) * bb.extent.y * 2.0f), 0.0f);

        Vector3 rt(bb.center.x + ((maxAnchor.x - 0.5f) * bb.extent.x * 2.0f),
                   bb.center.y + ((maxAnchor.y - 0.5f) * bb.extent.y * 2.0f), 0.0f);

        Vector3 rb(bb.center.x + ((maxAnchor.x - 0.5f) * bb.extent.x * 2.0f),
                   bb.center.y - ((0.5f - minAnchor.y) * bb.extent.y * 2.0f), 0.0f);

        Vector3Vector points = {lt, lt - cornerA, lt - cornerB,
                                lb, lb - cornerC, lb - cornerD,
                                rt, rt + cornerC, rt + cornerD,
                                rb, rb + cornerA, rb + cornerB};
        IndexVector indices = {0, 1, 1, 2, 2, 0,
                               3, 4, 4, 5, 5, 3,
                               6, 7, 7, 8, 8, 6,
                               9,10,10,11,11, 9};

        Gizmos::drawLines(points, indices, Vector4(1.0f));
    }

    m_box = rect->bound();
    if(!m_box.isValid()) {
        return;
    }

    bool isDrag = m_controller->isDrag();
    if(!isDrag) {
        m_position = objectPosition();
    }

    int axis;
    m_world = Handles::rectTool(m_box.center, m_box.extent * 2.0f, axis, isDrag);

    if(isDrag) {
        Vector3 delta(m_world - m_savedWorld);
        if(delta.length() > 1.0f) {
            for(const auto &it : qAsConst(m_selected)) {
                RectTransform *rect = static_cast<RectTransform *>(it.object->transform());

                Vector3 p(rect->position());
                Vector2 position(p.x, p.y);
                Vector2 size(rect->size());
                Vector2 pivot(rect->pivot());

                Vector2 min(position - (size * pivot));
                Vector2 max(position + (size * (Vector2(1.0f) - pivot)));

                if(Handles::s_Axes & Handles::POINT_T) {
                    max.y += delta.y;
                }
                if(Handles::s_Axes & Handles::POINT_B) {
                    min.y += delta.y;
                }
                if(Handles::s_Axes & Handles::POINT_L) {
                    min.x += delta.x;
                }
                if(Handles::s_Axes & Handles::POINT_R) {
                    max.x += delta.x;
                }

                size = Vector2(MAX(max.x - min.x, 0.0f), MAX(max.y - min.y, 0.0f));
                rect->setSize(size);

                if(rect->parentTransform()) {
                    rect->setPosition(Vector3(min + size * pivot, p.z));
                }
            }
        }

        m_savedWorld = m_world;
    }

    Qt::CursorShape shape = Qt::ArrowCursor;
    if(Handles::s_Axes == (Handles::POINT_T | Handles::POINT_B | Handles::POINT_L | Handles::POINT_R)) {
        shape = Qt::SizeAllCursor;
    } else if(Handles::s_Axes == (Handles::POINT_T | Handles::POINT_R)) {
        shape = Qt::SizeBDiagCursor;
    } else if(Handles::s_Axes == (Handles::POINT_T | Handles::POINT_L)) {
        shape = Qt::SizeFDiagCursor;
    } else if(Handles::s_Axes == (Handles::POINT_B | Handles::POINT_R)) {
        shape = Qt::SizeFDiagCursor;
    } else if(Handles::s_Axes == (Handles::POINT_B | Handles::POINT_L)) {
        shape = Qt::SizeBDiagCursor;
    } else if((Handles::s_Axes == Handles::POINT_T) | (Handles::s_Axes == Handles::POINT_B)) {
        shape = Qt::SizeVerCursor;
    } else if((Handles::s_Axes == Handles::POINT_L) | (Handles::s_Axes == Handles::POINT_R)) {
        shape = Qt::SizeHorCursor;
    }

    m_cursor = shape;
}

QString WidgetTool::icon() const {
    return ":/Images/editor/Transform.png";
}

QString WidgetTool::name() const {
    return "Resize";
}
