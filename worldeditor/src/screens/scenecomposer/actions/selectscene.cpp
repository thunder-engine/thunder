#include "selectscene.h"

#include "components/scene.h"
#include "components/world.h"

SelectScene::SelectScene(Scene *scene, ObjectController *ctrl, const TString &name, UndoCommand *group) :
        UndoCommand(name, group),
        m_controller(ctrl),
        m_object(scene->uuid()) {

}

void SelectScene::undo() {
    SelectScene::redo();
}

void SelectScene::redo() {
    uint32_t back = m_controller->scene()->uuid();

    Object *object = Engine::findObject(m_object);
    if(object && dynamic_cast<Scene *>(object)) {
        Engine::world()->setActiveScene(static_cast<Scene *>(object));
        m_object = back;
    }
}
