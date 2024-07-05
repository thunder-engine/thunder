#include "editor/editortool.h"

#include "components/actor.h"
#include "components/transform.h"
#include "components/renderable.h"
#include "components/camera.h"

#include "editor/viewport/handletools.h"

EditorTool::Select::Select() :
    uuid(0),
    object(nullptr),
    renderable(nullptr) {

}

EditorTool::EditorTool(EditorTool::SelectList &selection) :
    m_selected(selection),
    m_cursor(Qt::ArrowCursor),
    m_snap(0.0f) {

}

QString EditorTool::toolTip() const {
    return QString();
}

QString EditorTool::shortcut() const {
    return QString();
}

float EditorTool::snap() const {
    return m_snap;
}

void EditorTool::setSnap(float snap) {
    m_snap = snap;
}

void EditorTool::update(bool center, bool local, bool snap) {
    A_UNUSED(center);
    A_UNUSED(local);
    A_UNUSED(snap);
}

void EditorTool::beginControl() {
    m_propertiesCache.clear();

    for(auto &it : m_selected) {
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
}

void EditorTool::endControl() {

}

void EditorTool::cancelControl() {
    auto cache = m_propertiesCache.begin();
    for(auto &it : m_selected) {
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

Qt::CursorShape EditorTool::cursor() const {
    return m_cursor;
}

Vector3 EditorTool::objectPosition() {
    if(m_selected.size() == 1) {
        return m_selected.front().object->transform()->worldPosition();
    }
    return objectBound().center;
}

AABBox EditorTool::objectBound() {
    AABBox result;
    result.extent = Vector3(-1.0f);
    if(!m_selected.empty()) {
        bool first = true;
        for(auto &it : m_selected) {
            if(it.renderable == nullptr) {
                it.renderable = dynamic_cast<Renderable *>(it.object->component("Renderable"));
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

const VariantList &EditorTool::cache() const {
    return m_propertiesCache;
}
