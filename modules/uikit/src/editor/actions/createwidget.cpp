#include "createwidget.h"

#include <actor.h>
#include "components/widget.h"

CreateWidget::CreateWidget(const TString &type, Scene *scene, WidgetController *ctrl, UndoCommand *group) :
        UndoCommand(TString("Create ") + type, group),
        m_type(type),
        m_controller(ctrl) {

}

void CreateWidget::undo() {
    for(auto uuid : m_objects) {
        Object *object = Engine::findObject(uuid);
        if(object) {
            delete object;
        }
    }

    m_controller->clear(false);
    m_controller->selectActors(m_selected);

    emit m_controller->sceneUpdated();
}

void CreateWidget::redo() {
    m_objects.clear();
    m_selected.clear();

    for(auto it : m_controller->selected()) {
        m_selected.push_back(it->uuid());
    }

    Widget *root = m_controller->root();
    Object *parent = root->actor();
    if(!m_controller->selected().empty()) {
        parent = m_controller->selected().front();        
    }

    Object *object = Engine::composeActor(m_type, m_type, parent);
    if(object) {
        m_objects.push_back(object->uuid());
    }

    m_controller->clear(false);
    m_controller->selectActors(m_objects);

    emit m_controller->sceneUpdated();
}
