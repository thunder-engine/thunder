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
    if(m_effect && isEnabled() && !m_materials.empty()) {
        m_data.system[VisualEffect::DeltaTime] = dt;

        float &emitterAge = m_data.emitter[VisualEffect::EmitterAge];
        if(m_effect->continous() || emitterAge > 0.0f) {
            Matrix4 *t = reinterpret_cast<Matrix4 *>(&m_data.emitter[VisualEffect::Transform]);
            *t = transform()->worldTransform();

            m_effect->update(m_data);

            for(int i = 0; i < m_materials.size(); i++) {
                m_materials[i]->setInstanceCount(m_data.instances);

                memcpy(m_materials[i]->rawUniformBuffer().data(), m_data.render[i].data(), m_data.render[i].size());
            }
        }
    }
}
/*!
    \internal
*/
Mesh *EffectRender::meshToDraw(int instance) const {
    return m_effect ? m_effect->renderable(instance)->mesh : nullptr;
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
        EffectRender *render = static_cast<EffectRender *>(ptr);

        // Update materials
        for(auto it : render->m_materials) {
            delete it;
        }
        render->m_materials.clear();

        int capacity = render->m_effect->capacity();

        // Update system buffer
        render->m_data.system.resize(render->m_effect->systemStride());
        // Update emitter buffer
        render->m_data.emitter.resize(render->m_effect->emitterStride());
        // Update particles buffer
        render->m_data.particles.resize(capacity * render->m_effect->particleStride());

        render->m_data.render.resize(render->m_effect->renderablesCount());
        for(int i = 0; i < render->m_effect->renderablesCount(); i++) {
            const VisualEffect::Renderable *renderable = render->m_effect->renderable(i);
            if(renderable) {
                MaterialInstance *instance = nullptr;

                Material *m = renderable->material;
                if(m) {
                    instance = m->createInstance(renderable->type);

                    instance->setInstanceCount(capacity);

                    render->m_data.render[i].resize(capacity * (m->uniformSize() / sizeof(float)));
                }

                render->m_materials.push_back(instance);
            }
        }
    }
}
