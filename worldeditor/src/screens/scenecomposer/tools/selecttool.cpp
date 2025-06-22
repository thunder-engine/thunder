#include "selecttool.h"

#include <components/actor.h>
#include <components/transform.h>
#include <components/renderable.h>

#include <editor/viewport/handles.h>

#include <input.h>

#include "../actions/duplicateobjects.h"
#include "../actions/changeobjectproperty.h"

#include "../objectcontroller.h"

SelectTool::SelectTool(ObjectController *controller) :
        EditorTool(),
        m_controller(controller),
        m_snapEditor(nullptr),
        m_snap(0.0f)  {

}

void SelectTool::update(bool center, bool local, bool snap) {
    if(Input::isKeyDown(Input::KEY_DELETE)) {
        m_controller->onRemoveActor(m_controller->selected());
    }
}

void SelectTool::beginControl() {
    EditorTool::beginControl();

    if(Input::isKey(Input::KEY_LEFT_SHIFT)) {
        UndoManager::instance()->push(new DuplicateObjects(m_controller));
    }

    m_propertiesCache.clear();

    for(auto &it : m_controller->selectList()) {
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

void SelectTool::endControl() {
    UndoManager::instance()->beginGroup(name().c_str());

    auto cache = m_propertiesCache.begin();
    if(cache != m_propertiesCache.end()) {
        for(auto &it : m_controller->selectList()) {
            VariantMap components = (*cache).toMap();
            for(auto &child : it.object->getChildren()) {
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

                            UndoManager::instance()->push(new ChangeObjectProperty({component}, property.name(), value, m_controller, "", UndoManager::instance()->group()));
                        }
                    }
                }
            }

            ++cache;
        }
    }

    UndoManager::instance()->endGroup();
}

void SelectTool::cancelControl() {
    auto cache = m_propertiesCache.begin();
    if(cache != m_propertiesCache.end()) {
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
}

std::string SelectTool::icon() const {
    return ":/Images/editor/Select.png";
}

std::string SelectTool::name() const {
    return "Select";
}

std::string SelectTool::component() const {
    return Transform::metaClass()->name();
}

QLineEdit *SelectTool::snapWidget() {
    return m_snapEditor;
}

float SelectTool::snap() const {
    return m_snap;
}

void SelectTool::setSnap(float snap) {
    m_snap = snap;
}

Vector3 SelectTool::objectPosition() {
    if(m_controller->selectList().size() == 1) {
        return m_controller->selectList().front().object->transform()->worldPosition();
    }
    return objectBound().center;
}

AABBox SelectTool::objectBound() {
    AABBox result;
    result.extent = Vector3(-1.0f);
    if(!m_controller->selectList().empty()) {
        bool first = true;
        for(auto &it : m_controller->selectList()) {
            if(first) {
                result.center = it.object->transform()->worldPosition();
            } else {
                result.encapsulate(it.object->transform()->worldPosition());
            }
        }
    }
    return result;
}
