#include "selectobjects.h"

SelectObjects::SelectObjects(const std::list<uint32_t> &objects, ObjectController *ctrl, const TString &name, UndoCommand *group) :
        UndoCommand(name,group),
        m_objects(objects),
        m_controller(ctrl) {

}

void SelectObjects::undo() {
    SelectObjects::redo();
}

void SelectObjects::redo() {
    auto objects = m_controller->selected();

    m_controller->clear(false);
    m_controller->selectActors(m_objects);

    m_objects.clear();
    for(auto &it : objects) {
        m_objects.push_back(it->uuid());
    }
}
