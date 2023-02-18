#include "components/directlight.h"

#include "components/actor.h"
#include "components/transform.h"
#include "components/camera.h"

#include "resources/material.h"
#include "resources/mesh.h"
#include "resources/rendertarget.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"
#include "gizmos.h"

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
/*!
    \internal
*/
void DirectLight::drawGizmos() {
    Transform *t = transform();

    const int steps = 16;

    Vector3Vector vertices;
    vertices.reserve(steps * 2);
    vertices = Mathf::pointsArc(Quaternion(Vector3(1, 0, 0), 90), 0.25f, 0.0f, 360.0f, 16);
    for(int i = 0; i < steps; i++) {
        vertices.push_back(vertices[i] + Vector3(0.0f, 0.0f,-1.0f));
    }

    IndexVector indices;
    indices.resize((steps - 1) * 4);
    for(int i = 0; i < steps - 1; i++) {
        indices[i * 4] = i;
        indices[i * 4 + 1] = i+1;
        indices[i * 4 + 2] = i;
        indices[i * 4 + 3] = i+steps;
    }

    Gizmos::drawLines(vertices, indices, gizmoColor(), t->worldTransform());
    Gizmos::drawIcon(t->worldPosition(), Vector2(0.5f), ".embedded/directlight.png", color());
}
