#include "selectscene.h"

#include "components/scene.h"
#include "components/world.h"

SelectScene::SelectScene(Scene *scene, ObjectController *ctrl, const QString &name, QUndoCommand *group) :
        UndoCommand(name, ctrl, group),
        m_controller(ctrl),
        m_object(scene->uuid()) {

}

void SelectScene::undo() {
    SelectScene::redo();
}

void SelectScene::redo() {
    uint32_t back = m_controller->world()->activeScene()->uuid();

    Object *object = Engine::findObject(m_object);
    if(object && dynamic_cast<Scene *>(object)) {
        m_controller->world()->setActiveScene(static_cast<Scene *>(object));
        m_object = back;
    }
}
