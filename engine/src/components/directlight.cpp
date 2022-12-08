#include "components/directlight.h"

#include "components/actor.h"
#include "components/transform.h"
#include "components/camera.h"

#include "resources/material.h"
#include "resources/mesh.h"
#include "resources/rendertarget.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"

namespace {
const char *uni_direction = "uni.direction";
};

/*!
    \class DirectLight
    \brief The Directional Light simulates light that is being emitted from a source that is infinitely far away.
    \inmodule Engine

    To determine the emit direction DirectLight uses Transform component of the own Actor.
*/

DirectLight::DirectLight() {
    Material *material = Engine::loadResource<Material>(".embedded/DirectLight.shader");
    if(material) {
        MaterialInstance *instance = material->createInstance();

        setMaterial(instance);
    }
}
/*!
    \internal
*/
int DirectLight::lightType() const {
    return BaseLight::DirectLight;
}

#ifdef SHARED_DEFINE
#include "viewport/handles.h"

bool DirectLight::drawHandles(ObjectList &selected) {
    A_UNUSED(selected);
    Transform *t = transform();

    Matrix4 z(Vector3(), Quaternion(Vector3(1, 0, 0),-90), Vector3(1.0));
    Handles::s_Color = Handles::s_Second = color();
    Handles::drawArrow(Matrix4(t->worldPosition(), t->worldRotation(), Vector3(0.25f)) * z);
    bool result = Handles::drawBillboard(t->worldPosition(), Vector2(0.5f), Engine::loadResource<Texture>(".embedded/directlight.png"));
    Handles::s_Color = Handles::s_Second = Handles::s_Normal;

    return result;
}
#endif
