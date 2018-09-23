#include "controller.h"

#include "input.h"
#include "components/camera.h"

IController::IController() {
    m_pActiveCamera = nullptr;

    m_ObjectsList.clear();
}

void IController::update() {

}

Camera *IController::activeCamera() const {
    return m_pActiveCamera;
}

void IController::setActiveCamera(Camera *camera) {
    m_pActiveCamera = camera;
}

void IController::selectGeometry(Vector2 &pos, Vector2 &) {
    Vector4 result    = Input::instance()->mousePosition();
    pos = Vector2(result.x, result.y);
}

void IController::setSelectedObjects(const list<uint32_t> &id) {
    m_ObjectsList   = id;
}
