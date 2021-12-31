#include "resizetool.h"

#include <components/actor.h>
#include <components/transform.h>
#include <components/camera.h>
#include <components/renderable.h>

#include <editor/viewport/handles.h>

#include "objectctrl.h"

ResizeTool::ResizeTool(ObjectCtrl *controller, SelectList &selection) :
    SelectTool(controller, selection) {

}

void ResizeTool::beginControl() {
    SelectTool::beginControl();
    m_SavedBox = m_Box;
    for(auto &it : m_Selected) {
        if(it.renderable) {
            it.box = it.renderable->bound();

            Vector3 p = it.object->transform()->worldPosition();
            it.pivot = it.box.center - p;
        }
    }
}

void ResizeTool::update(bool pivot, bool local, float snap) {
    A_UNUSED(pivot);
    A_UNUSED(local);
    A_UNUSED(snap);

    m_Box = objectBound();
    if(!m_Box.isValid()) {
        return;
    }

    bool isDrag = m_pController->isDrag();
    if(!isDrag) {
        m_Position = objectPosition();
    }

    int axis;
    m_World = Handles::rectTool(m_Box.center, m_Box.extent * 2.0f, axis, isDrag);

    if(isDrag) {
        Vector3 delta;
        Vector3 mask(1.0f);

        if(Handles::s_Axes & Handles::POINT_R) {
            if(axis == Handles::AXIS_X) {
                delta.z = m_SavedWorld.z - m_World.z;
                mask.z = -1.0f;
            } else {
                delta.x = m_World.x - m_SavedWorld.x;
            }
        }
        if(Handles::s_Axes & Handles::POINT_L) {
            if(axis == Handles::AXIS_X) {
                delta.z = m_World.z - m_SavedWorld.z;
            } else {
                delta.x = m_SavedWorld.x - m_World.x;
                mask.x = -1.0f;
            }
        }

        if(Handles::s_Axes & Handles::POINT_T) {
            if(axis == Handles::AXIS_Y) {
                delta.z = m_World.z - m_SavedWorld.z;
            } else {
                delta.y = m_World.y - m_SavedWorld.y;
            }
        }
        if(Handles::s_Axes & Handles::POINT_B) {
            if(axis == Handles::AXIS_Y) {
                delta.z = m_SavedWorld.z - m_World.z;
                mask.z = -1.0f;
            } else {
                delta.y = m_SavedWorld.y - m_World.y;
                mask.y = -1.0f;
            }
        }
        delta *= 0.5f;

        for(const auto &it : m_Selected) {
            Transform *tr = it.object->transform();

            Matrix4 parent;
            if(tr->parentTransform()) {
                parent = tr->parentTransform()->worldTransform();
            }

            Vector3 p((parent * it.position) - m_Position);

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

                    tr->setPosition(parent.inverse() * (p + m_Position + delta * (mask - d)));
                }
            }

            if(!skipScale) {
                AABBox aabb(m_SavedBox.center + delta, m_SavedBox.extent + delta);
                Vector3 v(it.scale * Vector3((m_SavedBox.extent.x == 0.0f) ? 1.0f : (aabb.extent.x / m_SavedBox.extent.x),
                                             (m_SavedBox.extent.y == 0.0f) ? 1.0f : (aabb.extent.y / m_SavedBox.extent.y),
                                             (m_SavedBox.extent.z == 0.0f) ? 1.0f : (aabb.extent.z / m_SavedBox.extent.z)));

                tr->setScale(v);
                tr->setPosition(parent.inverse() * (v * p + m_Position + delta * mask));
            }
        }

        m_pController->objectsChanged(m_pController->selected(), "Scale");
        m_pController->objectsUpdated();
    }

    Qt::CursorShape shape = Qt::ArrowCursor;
    if(Handles::s_Axes == (Handles::POINT_T | Handles::POINT_R)) {
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
    m_Cursor = shape;
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
