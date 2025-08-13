#include "effectrender.h"

#include <algorithm>
#include <cstring>
#include <cfloat>

#include "actor.h"
#include "transform.h"
#include "camera.h"

#include "visualeffect.h"
#include "material.h"

#include "commandbuffer.h"
#include "timer.h"

/*!
    \class EffectRender
    \brief Draws a particle effect on the scene.
    \inmodule Components

    The ParticleRender component allows you to display Particle Effects such as fire and explosions.
*/

EffectRender::EffectRender() :
        m_effect(nullptr) {

}

EffectRender::~EffectRender() {
    if(m_effect) {
        m_effect->unsubscribe(this);
    }
}
/*!
    \internal
*/
void EffectRender::update() {
    deltaUpdate(Timer::deltaTime() * Timer::scale());
}
/*!
    \internal
*/
void EffectRender::deltaUpdate(float dt) {
    if(m_effect && isEnabled()) {
        Camera *camera = Camera::current();
        if(camera == nullptr || m_materials.empty()) {
            return;
        }

        float &emitterAge = m_emitterData[VisualEffect::EmitterAge];
        float &deltaTime = m_emitterData[VisualEffect::DeltaTime];

        deltaTime = dt;

        if(m_effect->continous() || emitterAge > 0.0f) {
            float &emitterSpawnCounter = m_emitterData[VisualEffect::SpawnCounter];

            emitterSpawnCounter += m_effect->spawnRate() * deltaTime;

            if(emitterAge > 0.0f) {
                emitterAge -= dt;
            }
        }

        m_effect->update(m_emitterData, m_particleData, m_renderData);

        int32_t count = static_cast<int32_t>(m_emitterData[VisualEffect::AliveParticles]);
        if(m_effect->local()) {
            Matrix4 world(transform()->worldTransform());

            int32_t stride = m_effect->renderableStride();
            for(int32_t i = 0; i < count; i++) {
                int index = i * stride;
                Vector3 p(world * Vector3(m_renderData[index + 12], m_renderData[index + 13], m_renderData[index + 14]));
                m_renderData[index + 12] = p.x;
                m_renderData[index + 13] = p.y;
                m_renderData[index + 14] = p.z;
            }
        }

        MaterialInstance *instance = m_materials.front();
        instance->setInstanceCount(count);

        memcpy(instance->rawUniformBuffer().data(), m_renderData.data(), m_renderData.size());
    }
}
/*!
    \internal
*/
Mesh *EffectRender::meshToDraw(int instance) const {
    A_UNUSED(instance);
    return m_effect ? m_effect->mesh() : nullptr;
}
/*!
    Returns a ParticleEffect assigned to the this component.
*/
VisualEffect *EffectRender::effect() const {
    return m_effect;
}
/*!
    Assgines a particle \a effect to the this component.
*/
void EffectRender::setEffect(VisualEffect *effect) {
    PROFILE_FUNCTION();

    if(m_effect) {
        m_effect->unsubscribe(this);
    }

    m_effect = effect;
    if(m_effect) {
        effectUpdated(Resource::Ready, this);
        m_effect->subscribe(&EffectRender::effectUpdated, this);
    }
}
/*!
    \internal
*/
AABBox EffectRender::localBound() const {
    if(m_effect) {
        m_effect->bound();
    }
    return Renderable::localBound();
}
/*!
    \internal
*/
void EffectRender::effectUpdated(int state, void *ptr) {
    if(state == Resource::Ready) {
        EffectRender *p = static_cast<EffectRender *>(ptr);

        // Update materials
        for(auto it : p->m_materials) {
            delete it;
        }
        p->m_materials.clear();

        int capacity = p->m_effect->capacity();

        if(p->m_effect->material()) {
            MaterialInstance *instance = p->m_effect->material()->createInstance(Material::Billboard);

            instance->setInstanceCount(capacity);

            p->m_materials.push_back(instance);
        }

        // Update emitter buffer
        p->m_emitterData.resize(VisualEffect::LastAttribute);
        // Update particles buffer
        p->m_particleData.resize(capacity * p->m_effect->particleStride());

        int renderableStride = p->m_effect->renderableStride();
        p->m_renderData.resize(capacity * renderableStride);

        Vector4 colorID(CommandBuffer::idToColor(p->actor()->uuid()));

        for(int i = 0; i < capacity; i++) {
            int r = i * renderableStride;

            p->m_renderData[r + 3] = colorID.x;
            p->m_renderData[r + 7] = colorID.y;
            p->m_renderData[r + 11] = colorID.z;
            p->m_renderData[r + 15] = colorID.w;
        }
    }
}
