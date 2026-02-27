#include "widgettool.h"

#include <gizmos.h>
#include <input.h>

#include <actor.h>
#include <transform.h>
#include <renderable.h>

#include <editor/viewport/handles.h>

#include "components/widget.h"
#include "components/recttransform.h"

#include "../widgetcontroller.h"
#include "../actions/changeproperty.h"

const Vector3 cornerA(10.0f, -5.0f, 0.0f);
const Vector3 cornerB( 5.0f,-10.0f, 0.0f);
const Vector3 cornerC(10.0f,  5.0f, 0.0f);
const Vector3 cornerD( 5.0f, 10.0f, 0.0f);

WidgetTool::WidgetTool(WidgetController *controller) :
        m_controller(controller),
        m_sensor(5.0f) {

}

void WidgetTool::beginControl() {
    EditorTool::beginControl();

    m_propertiesCache.clear();

    Actor *actor = static_cast<Actor *>(Engine::findObject(m_controller->selectedUuid()));
    RectTransform *rect = static_cast<RectTransform *>(actor->transform());

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

    m_position = rect->position();
    m_savedWorld = m_world;

    Vector2 pivot(rect->pivot());
    m_min = Vector2(m_position) - (rect->size() * pivot);
    m_max = Vector2(m_position) + (rect->size() * (Vector2(1.0f) - pivot));
}

void WidgetTool::endControl() {
    UndoCommand *group = new UndoCommand(name().c_str());

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

                    new ChangeProperty({component}, property.name(), value, m_controller, "", group);
                }
            }
        }
    }

    if(group->childCount() > 0) {
        m_controller->undoRedo()->push(group);
    }
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

Vector2 WidgetTool::recalcPosition(RectTransform *rect, RectTransform *root) const {
    Vector2 result;

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

    Vector2 boxSize(rect->size());
    Vector2 boxCenter(recalcPosition(rect, rootRect) + boxSize * 0.5f);

    bool isDrag = m_controller->isDrag();

    Handles::s_Color = Handles::s_Normal;

    int axis;
    m_world = Handles::rectTool(Vector3(boxCenter, 0.0f), Vector3(boxSize), axis, false, isDrag);

    if(isDrag) {
        Vector3 delta(m_world - m_savedWorld);
        if(delta.length() >= 1.0f) {
            Vector2 pivot(rect->pivot());
            Vector2 hint(rect->sizeHint());

            Vector2 min(m_min);
            Vector2 max(m_max);

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

            snapSolver(min, max, minAnchor, parent, parent, center);
            for(auto it : parent->children()) {
                if(it != rect) {
                    snapSolver(min, max, minAnchor, dynamic_cast<RectTransform *>(it), parent, center);
                }
            }

            Vector2 size(max - min);
            rect->setSize(size);

            if(rect->parentTransform()) {
                rect->setPosition(Vector3(min + size * pivot, m_position.z));
            }

            m_controller->sceneUpdated();
        }
    }

    Input::CursorShape shape = Input::CURSOR_ARROW;
    if(Handles::s_Axes == (Handles::TOP | Handles::BOTTOM | Handles::LEFT | Handles::RIGHT)) {
        shape = Input::CURSOR_ALLSIZE;
    } else if(Handles::s_Axes == (Handles::TOP | Handles::RIGHT)) {
        shape = Input::CURSOR_BDIAGSIZE;
    } else if(Handles::s_Axes == (Handles::TOP | Handles::LEFT)) {
        shape = Input::CURSOR_FDIAGSIZE;
    } else if(Handles::s_Axes == (Handles::BOTTOM | Handles::RIGHT)) {
        shape = Input::CURSOR_FDIAGSIZE;
    } else if(Handles::s_Axes == (Handles::BOTTOM | Handles::LEFT)) {
        shape = Input::CURSOR_BDIAGSIZE;
    } else if(Handles::s_Axes == Handles::TOP || Handles::s_Axes == Handles::BOTTOM) {
        shape = Input::CURSOR_VERSIZE;
    } else if(Handles::s_Axes == Handles::LEFT || Handles::s_Axes == Handles::RIGHT) {
        shape = Input::CURSOR_HORSIZE;
    }

    Input::mouseSetCursor(shape);
}

std::string WidgetTool::icon() const {
    return ":/Images/editor/Transform.png";
}

std::string WidgetTool::name() const {
    return "Widget Transform";
}

std::string WidgetTool::component() const {
    return Widget::metaClass()->name();
}

bool WidgetTool::snapHelperX(Vector2 &min, Vector2 &max, const Vector2 &point, bool isMove) const {
    float extent((max.x - min.x) * 0.5f);
    float center = min.x + extent;

    if(isMove && center > point.x - m_sensor && center < point.x + m_sensor) {
        min.x = point.x - extent;
        max.x = point.x + extent;
        return true;
    }

    if(min.x > point.x - m_sensor && min.x < point.x + m_sensor) {
        min.x = point.x;
        if(isMove) {
            max.x = point.x + extent * 2.0f;
        }
        return true;
    }

    if(max.x > point.x - m_sensor && max.x < point.x + m_sensor) {
        if(isMove) {
            min.x = point.x - extent * 2.0f;
        }
        max.x = point.x;
        return true;
    }

    return false;
}

bool WidgetTool::snapHelperY(Vector2 &min, Vector2 &max, const Vector2 &point, bool isMove) const {
    float extent((max.y - min.y) * 0.5f);
    float center = min.y + extent;

    if(isMove && center > point.y - m_sensor && center < point.y + m_sensor) {
        min.y = point.y - extent;
        max.y = point.y + extent;
        return true;
    }

    if(min.y > point.y - m_sensor && min.y < point.y + m_sensor) {
        min.y = point.y;
        if(isMove) {
            max.y = point.y + extent * 2.0f;
        }
        return true;
    }

    if(max.y > point.y - m_sensor && max.y < point.y + m_sensor) {
        if(isMove) {
            min.y = point.y - extent * 2.0f;
        }
        max.y = point.y;
        return true;
    }

    return false;
}

void WidgetTool::snapSolver(Vector2 &min, Vector2 &max, const Vector2 &minAnchor, RectTransform *rect, RectTransform *parent, const Vector2 &translation) const {
    if(rect) {
        Vector2 rectPosition(rect->position());
        rectPosition -= parent->position();
        Vector2 rectPivot(rect->pivot());
        Vector2 rectSize(rect->size());

        Vector2 rectMin(rectPosition - rectSize * rectPivot);
        Vector2 rectMax(rectPosition + rectSize * (Vector2(1.0f) - rectPivot));

        Vector2 shift;
        if(rect == parent) {
            shift = Vector2(rectSize.x * (0.5f - minAnchor.x),
                            rectSize.y * (0.5f - minAnchor.y));
        }

        bool isMove = Handles::s_Axes == (Handles::TOP | Handles::BOTTOM | Handles::LEFT | Handles::RIGHT);

        Vector2 rectExtent((rectMax - rectMin) * 0.5f);
        Vector2 rectCenter(rectMin + rectExtent);

        // Center X
        if(snapHelperX(min, max, rectCenter + shift, isMove)) {
            Vector2 v1(rectCenter.x, MIN(rectMin.y, min.y - shift.y));
            Vector2 v2(rectCenter.x, MAX(rectMax.y, max.y - shift.y));
            Gizmos::drawLines({Vector3(v1 + translation, 0.0f),
                               Vector3(v2 + translation, 0.0f)}, {0, 1}, Handles::s_zColor);
        }

        // Center Y
        if(snapHelperY(min, max, rectCenter + shift, isMove)) {
            Vector2 v1(MIN(rectMin.x, min.x - shift.x), rectCenter.y);
            Vector2 v2(MAX(rectMax.x, max.x - shift.x), rectCenter.y);
            Gizmos::drawLines({Vector3(v1 + translation, 0.0f),
                               Vector3(v2 + translation, 0.0f)}, {0, 1}, Handles::s_zColor);
        }

        // Min X
        if(snapHelperX(min, max, rectMin + shift, isMove)) {
            Vector2 v1(rectMin.x, MIN(rectMin.y, min.y - shift.y));
            Vector2 v2(rectMin.x, MAX(rectMax.y, max.y - shift.y));
            Gizmos::drawLines({Vector3(v1 + translation, 0.0f),
                               Vector3(v2 + translation, 0.0f)}, {0, 1}, Handles::s_zColor);
        }

        // Min Y
        if(snapHelperY(min, max, rectMin + shift, isMove)) {
            Vector2 v1(MIN(rectMin.x, min.x - shift.x), rectMin.y);
            Vector2 v2(MAX(rectMax.x, max.x - shift.x), rectMin.y);
            Gizmos::drawLines({Vector3(v1 + translation, 0.0f),
                               Vector3(v2 + translation, 0.0f)}, {0, 1}, Handles::s_zColor);
        }

        // Max X
        if(snapHelperX(min, max, rectMax + shift, isMove)) {
            Vector2 v1(rectMax.x, MIN(rectMin.y, min.y - shift.y));
            Vector2 v2(rectMax.x, MAX(rectMax.y, max.y - shift.y));
            Gizmos::drawLines({Vector3(v1 + translation, 0.0f),
                               Vector3(v2 + translation, 0.0f)}, {0, 1}, Handles::s_zColor);
        }

        // Max Y
        if(snapHelperY(min, max, rectMax + shift, isMove)) {
            Vector2 v1(MIN(rectMin.x, min.x - shift.x), rectMax.y);
            Vector2 v2(MAX(rectMax.x, max.x - shift.x), rectMax.y);
            Gizmos::drawLines({Vector3(v1 + translation, 0.0f),
                               Vector3(v2 + translation, 0.0f)}, {0, 1}, Handles::s_zColor);
        }
    }
}
