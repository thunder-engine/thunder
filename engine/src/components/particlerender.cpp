#include "particlerender.h"

#include <algorithm>
#include <cfloat>

#include "actor.h"
#include "transform.h"
#include "camera.h"

#include "particleeffect.h"
#include "material.h"

#include "commandbuffer.h"
#include "timer.h"

#define EFFECT "Effect"

/*!
    \class ParticleRender
    \brief Draws a particle effect on the scene.
    \inmodule Engine

    The ParticleRender component allows you to display Particle Effects such as fire and explosions.
*/

ParticleRender::ParticleRender() :
        m_effect(nullptr) {

}

ParticleRender::~ParticleRender() {
    if(m_effect) {
        m_effect->unsubscribe(this);
    }

    for(auto it : m_materials) {
        delete it;
    }
}
/*!
    \internal
*/
void ParticleRender::update() {
    deltaUpdate(Timer::deltaTime() * Timer::scale());
}
/*!
    \internal
*/
void ParticleRender::deltaUpdate(float dt) {
    Matrix4 m(transform()->worldTransform());

    Camera *camera = Camera::current();
    if(!camera) {
        return;
    }
    Vector3 pos(camera->transform()->worldPosition());

    if(m_effect) {
        m_aabb.setBox(Vector3(FLT_MAX), Vector3(-FLT_MAX));

        for(uint32_t index = 0; index < m_buffers.size(); index++) {
            ParticleEmitter *emitter = m_effect->emitter(index);

            m_visibleCount[index] = 0;
            uint32_t i = 0;
            bool local = emitter->local();
            bool continous = emitter->continous();

            // Spawn particles
            m_count[index] += emitter->distibution() * dt;
            while(isEnabled() && (continous || m_ejectionTime[index] > 0.0f) && m_count[index] >= 1.0f) {
                ParticleData particle;
                spawnParticle(*emitter, particle);
                particle.transform = m * particle.position;
                m_particles[index].push_back(particle);
                m_count[index] -= 1.0f;
            }

            if(m_buffers[index].size() < m_particles[index].size()) {
                m_buffers[index].resize(m_particles[index].size());
            }

            if(!continous) {
                m_ejectionTime[index] -= dt;
            }
            // Update particles
            for(auto &particle : m_particles[index]) {
                particle.life -= dt;
                if(particle.life >= 0.0f) {
                    updateParticle(*emitter, particle, dt);

                    particle.transform = (local) ? m * particle.position : particle.position;
                    particle.distance = (pos - particle.transform).sqrLength();

                    m_visibleCount[index]++;
                } else {
                    particle.distance = -1.0f;
                    if(isEnabled() && (continous || m_ejectionTime[index] > 0.0f) && m_count[index] >= 1.0f) {
                        spawnParticle(*emitter, particle);
                        particle.transform = m * particle.position;
                        m_count[index] -= 1.0f;
                    }
                }
                m_buffers[index][i].mat[0]  = particle.transform.x;
                m_buffers[index][i].mat[1]  = particle.transform.y;
                m_buffers[index][i].mat[2]  = particle.transform.z;

                m_buffers[index][i].mat[3]  = particle.angle.z;

                m_buffers[index][i].mat[4]  = particle.size.x;
                m_buffers[index][i].mat[5]  = particle.size.y;
                m_buffers[index][i].mat[6]  = particle.size.z;

                m_buffers[index][i].mat[7]  = particle.distance;

                m_buffers[index][i].mat[10] = particle.frame;
                m_buffers[index][i].mat[11] = particle.life;

                m_buffers[index][i].mat[12] = particle.color.x;
                m_buffers[index][i].mat[13] = particle.color.y;
                m_buffers[index][i].mat[14] = particle.color.z;
                m_buffers[index][i].mat[15] = particle.color.w;

                if(particle.life > 0.0f) {
                    m_aabb.encapsulate(particle.transform, particle.size.sqrLength());
                }

                i++;
            }

            std::sort(m_buffers[index].begin(), m_buffers[index].end(), [](const Matrix4 &left, const Matrix4 &right) { return left[7] > right[7]; });
        }
    }
}
/*!
    \internal
*/
void ParticleRender::draw(CommandBuffer &buffer, uint32_t layer) {
    Actor *a = actor();
    if(layer & a->layers()) {
        buffer.setObjectId(a->uuid());

        for(uint32_t index = 0; index < m_buffers.size(); index++) {
            if(m_visibleCount[index] > 0 && m_materials[index]) {
                buffer.setMaterialId(m_materials[index]->material()->uuid());

                ParticleEmitter *emitter = m_effect->emitter(index);
                buffer.drawMeshInstanced(m_buffers[index].data(), m_visibleCount[index], emitter->mesh(), 0, layer, m_materials[index]);
            }
        }
    }
}
/*!
    Returns a ParticleEffect assigned to the this component.
*/
ParticleEffect *ParticleRender::effect() const {
    return m_effect;
}
/*!
    Assgines a particle \a effect to the this component.
*/
void ParticleRender::setEffect(ParticleEffect *effect) {
    PROFILE_FUNCTION();

    if(effect) {
        if(m_effect) {
            m_effect->unsubscribe(this);
        }
        m_effect = effect;
        effectUpdated(ResourceState::Ready, this);
        m_effect->subscribe(&ParticleRender::effectUpdated, this);
    }
}
/*!
    \internal
*/
void ParticleRender::spawnParticle(ParticleEmitter &emitter, ParticleData &data) {
    PROFILE_FUNCTION();

    data.position.x = 0.0f;
    data.position.y = 0.0f;
    data.position.z = 0.0f;
    for(auto &it : emitter.modifiers()) {
        it->spawnParticle(data);
    }
}
/*!
    \internal
*/
void ParticleRender::updateParticle(ParticleEmitter &emitter, ParticleData &data, float dt) {
    PROFILE_FUNCTION();

    for(auto &it : emitter.modifiers()) {
        it->updateParticle(data, dt);
    }
}
/*!
    \internal
*/
AABBox ParticleRender::bound() const {
    return m_aabb;
}
/*!
    \internal
*/
void ParticleRender::loadUserData(const VariantMap &data) {
    PROFILE_FUNCTION();

    Component::loadUserData(data);

    auto it = data.find(EFFECT);
    if(it != data.end()) {
        setEffect(Engine::loadResource<ParticleEffect>((*it).second.toString()));
    }
}
/*!
    \internal
*/
VariantMap ParticleRender::saveUserData() const {
    PROFILE_FUNCTION();

    VariantMap result = Component::saveUserData();

    string ref = Engine::reference(m_effect);
    if(!ref.empty()) {
        result[EFFECT] = ref;
    }

    return result;
}
/*!
    \internal
*/
void ParticleRender::effectUpdated(int state, void *ptr) {
    if(state == ResourceState::Ready) {
        ParticleRender *p = static_cast<ParticleRender *>(ptr);

        p->m_buffers.clear();
        p->m_particles.clear();
        p->m_ejectionTime.clear();
        p->m_count.clear();
        p->m_visibleCount.clear();

        for(auto it : p->m_materials) {
            delete it;
        }

        int count = p->m_effect->emittersCount();

        p->m_buffers.resize(count);
        p->m_particles.resize(count);
        p->m_ejectionTime.resize(count);
        p->m_count.resize(count);
        p->m_visibleCount.resize(count);
        p->m_materials.resize(count);

        for(int32_t i = 0; i < p->m_effect->emittersCount(); i++) {
            ParticleEmitter *emitter = p->m_effect->emitter(i);
            if(emitter->material()) {
                p->m_materials[i] = emitter->material()->createInstance(Material::Billboard);
            }
        }
    }
}
