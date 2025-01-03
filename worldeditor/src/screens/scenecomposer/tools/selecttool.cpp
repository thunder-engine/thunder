#include "selecttool.h"

#include <QPushButton>

#include <components/actor.h>
#include <components/transform.h>
#include <components/renderable.h>

#include <editor/viewport/handles.h>

#include "../objectcontroller.h"

SelectTool::SelectTool(ObjectController *controller) :
        EditorTool(),
        m_controller(controller),
        m_snapEditor(nullptr),
        m_button(nullptr) {

}

void SelectTool::beginControl() {
    EditorTool::beginControl();

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

void SelectTool::cancelControl() {
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

QString SelectTool::icon() const {
    return ":/Images/editor/Select.png";
}

QString SelectTool::name() const {
    return "Select";
}

const VariantList &SelectTool::cache() const {
    return m_propertiesCache;
}

QPushButton *SelectTool::button() {
    if(m_button == nullptr) {
        m_button = new QPushButton();
        m_button->setProperty("blue", true);
        m_button->setProperty("checkred", true);
        m_button->setCheckable(true);
        m_button->setAutoExclusive(true);
        m_button->setIcon(QIcon(icon()));
        m_button->setObjectName(name());
        QString cut = shortcut();
        m_button->setShortcut(QKeySequence(cut));
        m_button->setToolTip(toolTip() + (!cut.isEmpty() ? (" (" + cut + ")") : ""));

        QObject::connect(m_button, &QPushButton::clicked, m_controller, &ObjectController::onChangeTool);
    }

    return m_button;
}

QLineEdit *SelectTool::snapWidget() {
    return m_snapEditor;
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
