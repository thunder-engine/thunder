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
    m_Selected(selection),
    m_Cursor(Qt::ArrowCursor) {

}

QString EditorTool::toolTip() const {
    return QString();
}

QString EditorTool::shortcut() const {
    return QString();
}

void EditorTool::update(bool pivot, bool local, float snap) {
    A_UNUSED(pivot);
    A_UNUSED(local);
    A_UNUSED(snap);

    Camera *cam = Camera::current();
    if(cam) {
        HandleTools::s_View = cam->viewMatrix();
        HandleTools::s_Projection = cam->projectionMatrix();
    }
}

void EditorTool::beginControl() {
    m_PropertiesCache.clear();

    for(auto &it : m_Selected) {
        Transform *t = it.object->transform();
        it.position = t->position();
        it.scale    = t->scale();
        it.euler    = t->rotation();

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
                components[to_string(component->uuid())] = properies;
            }
        }
        m_PropertiesCache.push_back(components);
    }
}

void EditorTool::endControl() {

}

void EditorTool::cancelControl() {
    auto cache = m_PropertiesCache.begin();
    for(auto &it : m_Selected) {
        VariantMap components = (*cache).toMap();
        for(auto &child : it.object->getChildren()) {
            Component *component = dynamic_cast<Component *>(child);
            if(component) {
                VariantMap properties = components[to_string(component->uuid())].toMap();
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
    return m_Cursor;
}

Vector3 EditorTool::objectPosition() {
    if(m_Selected.size() == 1) {
        return m_Selected.front().object->transform()->worldPosition();
    }
    return objectBound().center;
}

AABBox EditorTool::objectBound() {
    AABBox result;
    result.extent = Vector3(-1.0f);
    if(!m_Selected.empty()) {
        bool first = true;
        for(auto &it : m_Selected) {
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
    return m_PropertiesCache;
}
