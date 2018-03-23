#include "components/scene.h"

#include "components/actor.h"
#include "components/component.h"

Scene::Scene() :
    m_Ambient(0.3f) {

}

void Scene::update() {
    updateComponents(*this);
}

void Scene::updateComponents(Object &object) {
    for(auto &it : object.getChildren()) {
        Object *child  = it;
        Component *comp     = dynamic_cast<Component *>(child);
        if(comp) {
            comp->update();
        } else {
            Actor *actor    = dynamic_cast<Actor *>(child);
            if(actor) {
                if(!actor->isEnable()) {
                    continue;
                }
            }
            updateComponents(*child);
        }
    }
}

float Scene::ambient() const {
    return m_Ambient;
}

void Scene::setAmbient(float ambient) {
    m_Ambient   = ambient;
}
