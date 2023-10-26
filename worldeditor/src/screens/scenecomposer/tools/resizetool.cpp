#include "resizetool.h"

#include <components/actor.h>
#include <components/transform.h>
#include <components/camera.h>
#include <components/renderable.h>

#include <editor/viewport/handles.h>

#include "../objectcontroller.h"

ResizeTool::ResizeTool(ObjectController *controller, SelectList &selection) :
    SelectTool(controller, selection) {

}

void ResizeTool::beginControl() {
    SelectTool::beginControl();
    m_savedBox = m_box;
    for(auto &it : m_selected) {
        if(it.renderable) {
            it.box = it.renderable->bound();

            Vector3 p = it.object->transform()->worldPosition();
            it.pivot = it.box.center - p;
        }
    }
}

void ResizeTool::update(bool pivot, bool local, float snap) {
    SelectTool::update(pivot, local, snap);

    m_box = objectBound();
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
        Vector3 delta;
        Vector3 mask(1.0f);

        if(Handles::s_Axes & Handles::POINT_R) {
            if(axis == Handles::AXIS_X) {
                delta.z = m_savedWorld.z - m_world.z;
                mask.z = -1.0f;
            } else {
                delta.x = m_world.x - m_savedWorld.x;
            }
        }
        if(Handles::s_Axes & Handles::POINT_L) {
            if(axis == Handles::AXIS_X) {
                delta.z = m_world.z - m_savedWorld.z;
            } else {
                delta.x = m_savedWorld.x - m_world.x;
                mask.x = -1.0f;
            }
        }

        if(Handles::s_Axes & Handles::POINT_T) {
            if(axis == Handles::AXIS_Y) {
                delta.z = m_world.z - m_savedWorld.z;
            } else {
                delta.y = m_world.y - m_savedWorld.y;
            }
        }
        if(Handles::s_Axes & Handles::POINT_B) {
            if(axis == Handles::AXIS_Y) {
                delta.z = m_savedWorld.z - m_world.z;
                mask.z = -1.0f;
            } else {
                delta.y = m_savedWorld.y - m_world.y;
                mask.y = -1.0f;
            }
        }
        delta *= 0.5f;

        QSet<Scene *> scenes;

        for(const auto &it : qAsConst(m_selected)) {
            Transform *tr = it.object->transform();

            Matrix4 parent;
            if(tr->parentTransform()) {
                parent = tr->parentTransform()->worldTransform();
            }

            Vector3 p((parent * it.position) - m_position);

            bool skipScale = false;
            if(it.renderable) {
                const MetaObject *meta = it.renderable->metaObject();
                int index = meta->indexOfProperty("size");
                void *object = it.renderable;
                if(index == -1) {
                    meta = tr->metaObject();
                    index = meta->indexOfProperty("size");
                    object = tr;
                }
                if(index > -1) {
                    skipScale = true;
                    MetaProperty property = meta->property(index);
                    Vector2 size((it.box.extent.x + delta.x) * 2.0f / it.scale.x,
                                 (it.box.extent.y + delta.y) * 2.0f / it.scale.y);

                    property.write(object, size);

                    Vector3 d(it.pivot.x / ((it.box.extent.x == 0.0f) ? 1.0f : it.box.extent.x),
                              it.pivot.y / ((it.box.extent.y == 0.0f) ? 1.0f : it.box.extent.y),
                              it.pivot.z / ((it.box.extent.z == 0.0f) ? 1.0f : it.box.extent.z));

                    tr->setPosition(parent.inverse() * (p + m_position + delta * (mask - d)));
                }
            }

            if(!skipScale) {
                AABBox aabb(m_savedBox.center + delta, m_savedBox.extent + delta);
                Vector3 v(it.scale * Vector3((m_savedBox.extent.x == 0.0f) ? 1.0f : (aabb.extent.x / m_savedBox.extent.x),
                                             (m_savedBox.extent.y == 0.0f) ? 1.0f : (aabb.extent.y / m_savedBox.extent.y),
                                             (m_savedBox.extent.z == 0.0f) ? 1.0f : (aabb.extent.z / m_savedBox.extent.z)));

                tr->setScale(v);
                tr->setPosition(parent.inverse() * (v * p + m_position + delta * mask));
            }

            scenes.insert(it.object->scene());
        }
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

QString ResizeTool::icon() const {
    return ":/Images/editor/Transform.png";
}

QString ResizeTool::name() const {
    return "Resize";
}

QString ResizeTool::toolTip() const {
    return QObject::tr("Select and manipulate objects with rect transform tool");
}

QString ResizeTool::shortcut() const {
    return "Shift+G";
}
