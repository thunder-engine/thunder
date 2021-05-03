#include "system.h"

#include <components/component.h>

System::System() :
    m_pScene(nullptr) {

}

void System::syncSettings() const {

}

void System::composeComponent(Component *component) const {
    A_UNUSED(component);
}

void System::setActiveScene(Scene *scene) {
    m_pScene = scene;
}

void System::processEvents() {
    ObjectSystem::processEvents();

    update(m_pScene);
}
