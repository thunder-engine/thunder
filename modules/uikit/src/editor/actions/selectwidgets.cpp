#include "selectwidgets.h"

SelectWidgets::SelectWidgets(const std::list<uint32_t> &objects, WidgetController *ctrl, const TString &name, UndoCommand *group) :
        UndoCommand(name, group),
        m_objects(objects),
        m_controller(ctrl) {

}

void SelectWidgets::undo() {
    SelectWidgets::redo();
}

void SelectWidgets::redo() {
    Object::ObjectList objects = m_controller->selected();

    m_controller->clear(false);
    m_controller->selectActors(m_objects);

    m_objects.clear();
    for(auto &it : objects) {
        m_objects.push_back(it->uuid());
    }
}
