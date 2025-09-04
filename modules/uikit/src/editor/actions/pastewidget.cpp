#include "pastewidget.h"

#include <components/component.h>

PasteWidget::PasteWidget(WidgetController *ctrl, const TString &name, UndoCommand *group) :
        UndoCommand(name, group),
        m_data(ctrl->copyData()),
        m_controller(ctrl) {

}

void PasteWidget::undo() {
    for(auto it : m_objects) {
        Object *object = Engine::findObject(it);
        if(object) {
            delete object;
        }
    }

    m_controller->clear(false);
    m_controller->selectActors(m_selected);

    emit m_controller->sceneUpdated();
}

void PasteWidget::redo() {
    m_objects.clear();
    m_selected.clear();

    for(auto it : m_controller->selected()) {
        m_selected.push_back(it->uuid());
    }

    Engine::blockObjectCache(true);
    Object *object = Engine::toObject(m_data);
    Engine::blockObjectCache(false);

    Object::ObjectList objects;
    Object::enumObjects(object, objects);

    for(auto it : objects) {
        uint32_t oldUuid = it->uuid();

        Engine::blockObjectCache(true);
        Engine::replaceUUID(it, Engine::generateUUID());
        Engine::blockObjectCache(false);

        auto uuid = m_uuidBinds.find(oldUuid);
        if(uuid == m_uuidBinds.end()) {
            uint32_t newUuid = Engine::generateUUID();
            Engine::replaceUUID(it, newUuid);

            m_uuidBinds[oldUuid] = newUuid;
        } else {
            Engine::replaceUUID(it, uuid->second);
        }
    }

    for(auto it : objects) {
        const MetaObject *meta = it->metaObject();
        for(int i = 0; i < meta->propertyCount(); i++) {
            MetaProperty property = meta->property(i);

            TString editor = UiEdit::propertyTag(property, "editor=");
            if(editor == "Component") {
                Variant value = property.read(it);
                Component *component = (value.data() == nullptr) ? nullptr : *(reinterpret_cast<Component **>(value.data()));
                if(component) {
                    auto componentUuid = m_uuidBinds.find(component->uuid());
                    if(componentUuid != m_uuidBinds.end()) {
                        Object *newComponent = Engine::findObject(componentUuid->second);
                        if(newComponent) {
                            property.write(it, Variant(value.userType(), &newComponent));
                        }
                    }
                }
            }
        }
    }

    m_objects.push_back(object->uuid());

    m_controller->clear(false);
    m_controller->selectActors(m_objects);

    emit m_controller->sceneUpdated();
}
