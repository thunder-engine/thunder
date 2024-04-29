#include "particlerender.h"

#include <algorithm>
#include <cstring>
#include <cfloat>

#include "actor.h"
#include "transform.h"
#include "camera.h"

#include "particleeffect.h"
#include "material.h"

#include "commandbuffer.h"
#include "timer.h"

namespace {
    const char *gEffect = "Effect";
};

/*!
    \class ParticleRender
    \brief Draws a particle effect on the scene.
    \inmodule Components

    The ParticleRender component allows you to display Particle Effects such as fire and explosions.
*/

ParticleRender::ParticleRender() :
        m_ejectionTime(0.0f),
        m_count(0.0f),
        m_effect(nullptr) {

}

ParticleRender::~ParticleRender() {
    if(m_effect) {
        m_effect->unsubscribe(this);
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

        bool continous = m_effect->continous();

        // Spawn particles
        m_count += m_effect->distribution() * dt;
        while(isEnabled() && (continous || m_ejectionTime > 0.0f) && m_count >= 1.0f) {
            spawnParticle(m);

            m_count -= 1.0f;
        }

        if(!continous) {
            m_ejectionTime -= dt;
        }

        // Update particles
        bool local = m_effect->local();

        uint32_t visibleCount = 0;
        for(auto &particle : m_particles) {
            particle.life -= dt;
            if(particle.life >= 0.0f) {
                updateParticle(particle, dt);

                particle.transform = (local) ? m * particle.position : particle.position;
                particle.distance = (pos - particle.transform).sqrLength();

                visibleCount++;
            }

            if(particle.life > 0.0f) {
                m_aabb.encapsulate(particle.transform, particle.size.sqrLength());
            }
        }

        std::sort(m_particles.begin(), m_particles.end(), [](const ParticleData &left, const ParticleData &right) { return left.distance > right.distance; });

        MaterialInstance &instance = *m_materials.front();

        instance.setInstanceCount(visibleCount);
        vector<uint8_t> &uniformBuffer = instance.rawUniformBuffer();

        Vector4 colorID(CommandBuffer::idToColor(actor()->uuid()));

        uint32_t offset = 0;
        for(auto &particle : m_particles) {
            if(particle.life > 0.0f) {
                Vector4 v0(particle.transform, particle.angle.z);
                memcpy(&uniformBuffer[offset], &v0, sizeof(Vector4));
                offset += sizeof(Vector4);

                Vector4 v1(particle.size, particle.distance);
                memcpy(&uniformBuffer[offset], &v1, sizeof(Vector4));
                offset += sizeof(Vector4) * 2;

                memcpy(&uniformBuffer[offset], &colorID, sizeof(Vector4));
                offset += sizeof(Vector4);

                memcpy(&uniformBuffer[offset], &particle.color, sizeof(Vector4));
                offset += sizeof(Vector4);
            }
        }
    }
}
/*!
    \internal
*/
void ParticleRender::draw(CommandBuffer &buffer, uint32_t layer) {
    Actor *a = actor();
    if(layer & a->layers()) {
        MaterialInstance *instance = m_materials.front();
        if(instance && instance->instanceCount() > 0) {
            buffer.drawMesh(m_effect->mesh(), 0, layer, *instance);
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

    if(m_effect != effect) {
        if(m_effect) {
            m_effect->unsubscribe(this);
        }

        m_effect = effect;
        if(m_effect) {
            effectUpdated(Resource::Ready, this);
            m_effect->subscribe(&ParticleRender::effectUpdated, this);
        }
    }
}
/*!
    \internal
*/
void ParticleRender::spawnParticle(Matrix4 &matrix) {
    PROFILE_FUNCTION();

    ParticleData *particle = nullptr;
    for(auto &it : m_particles) {
        if(it.life <= 0.0f) {
            particle = &it;
            break;
        }
    }

    if(particle == nullptr) {
        m_particles.push_back(ParticleData());
        particle = &m_particles.back();
    }

    if(particle) {
        for(auto &it : m_effect->modifiers()) {
            it->spawnParticle(*particle);
        }

        particle->transform = matrix * particle->position;
    }
}
/*!
    \internal
*/
void ParticleRender::updateParticle(ParticleData &data, float dt) {
    PROFILE_FUNCTION();

    for(auto &it : m_effect->modifiers()) {
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

    auto it = data.find(gEffect);
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
        result[gEffect] = ref;
    }

    return result;
}
/*!
    \internal
*/
void ParticleRender::effectUpdated(int state, void *ptr) {
    if(state == Resource::Ready) {
        ParticleRender *p = static_cast<ParticleRender *>(ptr);

        p->m_particles.clear();

        for(auto it : p->m_materials) {
            delete it;
        }

        if(p->m_effect->material()) {
            p->m_materials.push_back(p->m_effect->material()->createInstance(Material::Billboard));
            // Need to map memory here
        }
    }
}
