#include "pasteobjects.h"

#include <components/world.h>

PasteObjects::PasteObjects(ObjectController *ctrl, const TString &name, UndoCommand *group) :
        UndoCommand(name, group),
        m_data(ctrl->copyData()),
        m_controller(ctrl) {

}

void PasteObjects::undo() {
    for(auto it : m_uuidPairs) {
        Object *object = Engine::findObject(it.second);
        if(object) {
            delete object;
        }
    }

    emit m_controller->sceneUpdated(m_controller->world()->activeScene());
}

void PasteObjects::redo() {
    Object::ObjectList objects;

    for(auto dataIt : m_data) {
        Engine::blockObjectCache(true);
        Object *object = Engine::toObject(dataIt);
        Engine::blockObjectCache(false);

        Object::enumObjects(object, objects);
    }

    if(m_uuidPairs.empty()) {
        for(auto it : objects) {
            uint32_t oldUuid = it->uuid();

            Engine::blockObjectCache(true);
            Engine::replaceUUID(it, Engine::generateUUID());
            Engine::blockObjectCache(false);

            uint32_t newUuid = Engine::generateUUID();
            Engine::replaceUUID(it, newUuid);

            m_uuidPairs[oldUuid] = newUuid;
        }
    } else {
        for(auto it : objects) {
            uint32_t oldUuid = it->uuid();

            Engine::blockObjectCache(true);
            Engine::replaceUUID(it, Engine::generateUUID());
            Engine::blockObjectCache(false);

            Engine::replaceUUID(it, m_uuidPairs[oldUuid]);
        }
    }

    emit m_controller->sceneUpdated(m_controller->world()->activeScene());
}
