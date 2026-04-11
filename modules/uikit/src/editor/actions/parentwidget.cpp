#include "parentwidget.h"

ParentWidget::ParentWidget(const Object::ObjectList &objects, Object *parent, int32_t position, WidgetController *ctrl, const TString &name, UndoCommand *group) :
        UndoCommand(name, group),
        m_parent(parent->uuid()),
        m_position(position),
        m_controller(ctrl) {

    for(auto it : objects) {
        m_objects.push_back(it->uuid());
    }
}

void ParentWidget::undo() {
    for(auto it : m_objects) {
        Object *object = Engine::findObject(it);
        if(object) {
            auto parentIt = m_parentCache.find(object->uuid());
            if(parentIt != m_parentCache.end()) {
                Object *parent = Engine::findObject(parentIt->second);
                if(parent) {
                    object->setParent(parent);
                }
            }
        }
    }

    emit m_controller->sceneUpdated();
}

void ParentWidget::redo() {
    m_parentCache.clear();

    for(auto it : m_objects) {
        Object *object = Engine::findObject(it);
        if(object) {
            m_parentCache[object->uuid()] = object->parent()->uuid();

            Object *parent = Engine::findObject(m_parent);
            if(parent) {
                object->setParent(parent, m_position);
            }
        }
    }

    emit m_controller->sceneUpdated();
}
