#include "system.h"

ISystem::ISystem() :
    m_pScene(nullptr) {

}

void ISystem::setActiveScene(Scene *scene) {
    m_pScene = scene;
}

void ISystem::processEvents() {
    ObjectSystem::processEvents();

    update(m_pScene);
}
