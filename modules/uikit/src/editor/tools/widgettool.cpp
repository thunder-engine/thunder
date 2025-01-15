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

WidgetTool::WidgetTool(WidgetController *controller) :
        m_controller(controller) {

}

void WidgetTool::beginControl() {
    EditorTool::beginControl();

    m_propertiesCache.clear();

    for(auto &it : m_controller->selectList()) {
        Transform *t = it.object->transform();
        it.position = t->position();
        it.scale    = t->scale();
        it.euler    = t->rotation();
        it.quat     = t->quaternion();

        VariantMap components;
        for(auto &child : it.object->getChildren()) {
            Component *component = dynamic_cast<Component *>(child);
            if(component) {
                VariantMap properies;
                const MetaObject *meta = component->metaObject();
                for(int i = 0; i < meta->propertyCount(); i++) {
                    MetaProperty property = meta->property(i);
                    properies[property.name()] = property.read(component);
                }
                components[std::to_string(component->uuid())] = properies;
            }
        }
        m_propertiesCache.push_back(components);
    }

    m_position = objectPosition();
    m_savedWorld = m_world;
}

void WidgetTool::cancelControl() {
    auto cache = m_propertiesCache.begin();
    for(auto &it : m_controller->selectList()) {
        VariantMap components = (*cache).toMap();
        for(auto &child : it.object->getChildren()) {
            Component *component = dynamic_cast<Component *>(child);
            if(component) {
                VariantMap properties = components[std::to_string(component->uuid())].toMap();
                const MetaObject *meta = component->metaObject();
                for(int i = 0; i < meta->propertyCount(); i++) {
                    MetaProperty property = meta->property(i);
                    property.write(component, properties[property.name()]);
                }
            }
        }

        ++cache;
    }
}

void WidgetTool::update(bool pivot, bool local, bool snap) {
    EditorTool::update(pivot, local, snap);

    RectTransform *rect = static_cast<RectTransform *>(m_controller->selectList().front().object->transform());

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
    m_world = Handles::rectTool(m_box.center, m_box.extent * 2.0f, axis, true, isDrag);

    if(isDrag) {
        Vector3 delta(m_world - m_savedWorld);
        if(delta.length() > 1.0f) {
            for(const auto &it : qAsConst(m_controller->selectList())) {
                RectTransform *rect = static_cast<RectTransform *>(it.object->transform());

                Vector3 p(rect->position());
                Vector2 position(p.x, p.y);
                Vector2 size(rect->size());
                Vector2 pivot(rect->pivot());

                Vector2 min(position - (size * pivot));
                Vector2 max(position + (size * (Vector2(1.0f) - pivot)));

                if(Handles::s_Axes & Handles::TOP) {
                    max.y += delta.y;
                }
                if(Handles::s_Axes & Handles::BOTTOM) {
                    min.y += delta.y;
                }
                if(Handles::s_Axes & Handles::LEFT) {
                    min.x += delta.x;
                }
                if(Handles::s_Axes & Handles::RIGHT) {
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
    if(Handles::s_Axes == (Handles::TOP | Handles::BOTTOM | Handles::LEFT | Handles::RIGHT)) {
        shape = Qt::SizeAllCursor;
    } else if(Handles::s_Axes == (Handles::TOP | Handles::RIGHT)) {
        shape = Qt::SizeBDiagCursor;
    } else if(Handles::s_Axes == (Handles::TOP | Handles::LEFT)) {
        shape = Qt::SizeFDiagCursor;
    } else if(Handles::s_Axes == (Handles::BOTTOM | Handles::RIGHT)) {
        shape = Qt::SizeFDiagCursor;
    } else if(Handles::s_Axes == (Handles::BOTTOM | Handles::LEFT)) {
        shape = Qt::SizeBDiagCursor;
    } else if((Handles::s_Axes == Handles::TOP) | (Handles::s_Axes == Handles::BOTTOM)) {
        shape = Qt::SizeVerCursor;
    } else if((Handles::s_Axes == Handles::LEFT) | (Handles::s_Axes == Handles::RIGHT)) {
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

const VariantList &WidgetTool::cache() const {
    return m_propertiesCache;
}

Vector3 WidgetTool::objectPosition() {
    if(m_controller->selectList().size() == 1) {
        return m_controller->selectList().front().object->transform()->worldPosition();
    }
    return objectBound().center;
}

AABBox WidgetTool::objectBound() {
    AABBox result;
    result.extent = Vector3(-1.0f);
    if(!m_controller->selectList().empty()) {
        bool first = true;
        for(auto &it : m_controller->selectList()) {
            if(it.renderable == nullptr) {
                it.renderable = it.object->getComponent<Renderable>();
            }
            if(it.renderable) {
                if(first) {
                    result = it.renderable->bound();
                    first = false;
                } else {
                    result.encapsulate(it.renderable->bound());
                }
            } else {
                if(first) {
                    result.center = it.object->transform()->worldPosition();
                } else {
                    result.encapsulate(it.object->transform()->worldPosition());
                }
            }
        }
    }
    return result;
}
