#include "components/scene.h"

#include "components/actor.h"
#include "components/component.h"

Scene::Scene() :
    m_Ambient(0.2f) {

}

float Scene::ambient() const {
    return m_Ambient;
}

void Scene::setAmbient(float ambient) {
    m_Ambient   = ambient;
}
