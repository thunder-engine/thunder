#include "widgettool.h"

#include <gizmos.h>

#include <actor.h>
#include <transform.h>
#include <renderable.h>

#include <editor/viewport/handles.h>

#include "components/widget.h"
#include "components/recttransform.h"

#include "../widgetcontroller.h"

const Vector3 cornerA(10.0f, -5.0f, 0.0f);
const Vector3 cornerB( 5.0f,-10.0f, 0.0f);
const Vector3 cornerC(10.0f,  5.0f, 0.0f);
const Vector3 cornerD( 5.0f, 10.0f, 0.0f);

WidgetTool::WidgetTool(WidgetController *controller) :
        m_controller(controller) {

}

void WidgetTool::beginControl() {
    EditorTool::beginControl();

    m_propertiesCache.clear();

    Actor *actor = static_cast<Actor *>(Engine::findObject(m_controller->selectedUuid()));

    VariantMap components;
    for(auto &child : actor->getChildren()) {
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


    m_position = objectPosition();
    m_savedWorld = m_world;
}

void WidgetTool::endControl() {
    UndoManager::instance()->beginGroup(name().c_str());

    auto cache = m_propertiesCache.begin();

    Actor *actor = static_cast<Actor *>(Engine::findObject(m_controller->selectedUuid()));

    VariantMap components = (*cache).toMap();
    for(auto &child : actor->getChildren()) {

        Component *component = dynamic_cast<Component *>(child);
        if(component) {
            VariantMap properties = components[std::to_string(component->uuid())].toMap();
            const MetaObject *meta = component->metaObject();
            for(int i = 0; i < meta->propertyCount(); i++) {
                MetaProperty property = meta->property(i);

                Variant value = property.read(component);
                Variant data = properties[property.name()];
                if(value != data) {
                    property.write(component, data);

                    UndoManager::instance()->push(new ChangeProperty({component}, property.name(), value, m_controller, "", UndoManager::instance()->group()));
                }
            }
        }
    }

    UndoManager::instance()->endGroup();
}

void WidgetTool::cancelControl() {
    auto cache = m_propertiesCache.begin();

    Actor *actor = static_cast<Actor *>(Engine::findObject(m_controller->selectedUuid()));

    VariantMap components = (*cache).toMap();
    for(auto &child : actor->getChildren()) {
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
}

Vector3 WidgetTool::recalcPosition(RectTransform *rect, RectTransform *root) const {
    Vector3 result;

    RectTransform *it = rect;
    while(it && it != root) {
        result += it->position();

        RectTransform *parentRect = dynamic_cast<RectTransform *>(it->parentTransform());
        if(parentRect) {
            Vector2 anchors(it->minAnchors() + it->maxAnchors());
            Vector2 parentCenter(anchors * parentRect->size() * 0.5f);

            float x;
            if(abs(it->minAnchors().x - it->maxAnchors().x) > EPSILON) { // fit to parent
                result.x += parentRect->size().x * it->minAnchors().x + it->margin().w;
            } else {
                result.x += parentCenter.x - it->size().x * it->pivot().x + it->margin().w;
            }

            float y;
            if(abs(it->minAnchors().y - it->maxAnchors().y) > EPSILON) { // fit to parent
                result.y += parentRect->size().y * it->minAnchors().y + it->margin().z;
            } else {
                result.y += parentCenter.y - it->size().y * it->pivot().y + it->margin().z;
            }
        }

        it = parentRect;
    }

    return result;
}

void WidgetTool::update(bool pivot, bool local, bool snap) {
    EditorTool::update(pivot, local, snap);

    Actor *actor = static_cast<Actor *>(Engine::findObject(m_controller->selectedUuid()));
    RectTransform *rect = static_cast<RectTransform *>(actor->transform());

    // Anchor cross
    RectTransform *parent = dynamic_cast<RectTransform *>(rect->parentTransform());
    if(parent == nullptr) {
        return;
    }

    Widget *root = m_controller->root();
    RectTransform *rootRect(root->rectTransform());

    Vector3 size(parent->size(), 0.0f);
    Vector3 center(recalcPosition(parent, rootRect) + size * 0.5f);

    Vector2 minAnchor(rect->minAnchors());
    Vector2 maxAnchor(rect->maxAnchors());

    Vector3 lt(center.x - (0.5f - minAnchor.x) * size.x,
               center.y + (maxAnchor.y - 0.5f) * size.y, 0.0f);

    Vector3 lb(center.x - (0.5f - minAnchor.x) * size.x,
               center.y - (0.5f - minAnchor.y) * size.y, 0.0f);

    Vector3 rt(center.x + (maxAnchor.x - 0.5f) * size.x,
               center.y + (maxAnchor.y - 0.5f) * size.y, 0.0f);

    Vector3 rb(center.x + (maxAnchor.x - 0.5f) * size.x,
               center.y - (0.5f - minAnchor.y) * size.y, 0.0f);

    Vector3Vector points = {lt, lt - cornerA, lt - cornerB,
                            lb, lb - cornerC, lb - cornerD,
                            rt, rt + cornerC, rt + cornerD,
                            rb, rb + cornerA, rb + cornerB};
    IndexVector indices = {0, 1, 1, 2, 2, 0,
                           3, 4, 4, 5, 5, 3,
                           6, 7, 7, 8, 8, 6,
                           9,10,10,11,11, 9};

    Gizmos::drawLines(points, indices, Vector4(1.0f));

    Vector3 boxSize(rect->size(), 0.0f);
    Vector3 boxCenter(recalcPosition(rect, rootRect) + boxSize * Vector3(rect->pivot(), 0.0f));

    bool isDrag = m_controller->isDrag();

    int axis;
    m_world = Handles::rectTool(boxCenter, boxSize, axis, false, isDrag);

    if(isDrag) {
        Vector3 delta(m_world - m_savedWorld);
        if(delta.length() > 1.0f) {
            Actor *actor = static_cast<Actor *>(Engine::findObject(m_controller->selectedUuid()));
            RectTransform *rect = static_cast<RectTransform *>(actor->transform());

            Vector3 p(rect->position());
            Vector2 position(p.x, p.y);
            Vector2 size(rect->size());
            Vector2 pivot(rect->pivot());
            Vector2 hint(rect->sizeHint());

            Vector2 localMin(size * pivot);
            Vector2 localMax(size * (Vector2(1.0f) - pivot));
            Vector2 min(position - localMin);
            Vector2 max(position + localMax);

            bool moveAll = Handles::s_Axes == (Handles::TOP | Handles::BOTTOM | Handles::LEFT | Handles::RIGHT);

            if(Handles::s_Axes & Handles::TOP) {
                max.y += delta.y;
                float limit = min.y + hint.y;
                if(!moveAll && max.y < limit) {
                    max.y = limit;
                }
            }
            if(Handles::s_Axes & Handles::BOTTOM) {
                min.y += delta.y;
                float limit = max.y - hint.y;
                if(!moveAll && min.y > limit) {
                    min.y = limit;
                }
            }
            if(Handles::s_Axes & Handles::LEFT) {
                min.x += delta.x;
                float limit = max.x - hint.x;
                if(!moveAll && min.x > limit) {
                    min.x = limit;
                }
            }
            if(Handles::s_Axes & Handles::RIGHT) {
                max.x += delta.x;
                float limit = min.x + hint.x;
                if(!moveAll && max.x < limit) {
                    max.x = limit;
                }
            }

            size = Vector2(max.x - min.x, max.y - min.y);
            rect->setSize(size);

            if(rect->parentTransform()) {
                rect->setPosition(Vector3(min + size * pivot, p.z));
            }

            m_savedWorld = m_world;
        }
    } else {
        m_position = objectPosition();
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
    } else if(Handles::s_Axes == Handles::TOP || Handles::s_Axes == Handles::BOTTOM) {
        shape = Qt::SizeVerCursor;
    } else if(Handles::s_Axes == Handles::LEFT || Handles::s_Axes == Handles::RIGHT) {
        shape = Qt::SizeHorCursor;
    }

    m_cursor = shape;
}

std::string WidgetTool::icon() const {
    return ":/Images/editor/Transform.png";
}

std::string WidgetTool::name() const {
    return "Resize";
}

std::string WidgetTool::component() const {
    return Widget::metaClass()->name();
}

Vector3 WidgetTool::objectPosition() {
    Actor *actor = static_cast<Actor *>(Engine::findObject(m_controller->selectedUuid()));
    return actor->transform()->worldPosition();
}
