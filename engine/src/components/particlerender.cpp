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
    const char *gEffect("Effect");
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
    if(m_effect && isEnabled()) {
        Camera *camera = Camera::current();
        if(camera == nullptr || m_materials.empty()) {
            return;
        }

        bool local = m_effect->local();
        bool continous = m_effect->continous();

        if(continous || m_ejectionTime > 0.0f) {
            m_count += m_effect->distribution() * dt;

            for(int i = 0; i < m_particles.size(); i++) {
                if(m_count >= 1.0f) {
                    ParticleTransientData &p = m_particles[i];
                    if(p.life <= 0.0f) {
                        p.position = Vector3();
                        p.size = Vector3(1.0f);
                        p.color = Vector4(1.0f);
                        p.uv = Vector4(1.0f, 1.0f, 0.0f, 0.0f);

                        for(auto &it : m_effect->modificators()) {
                            it->spawnParticle(p, i);
                        }

                        p.life = p.lifetime;

                        m_count -= 1.0f;
                    }
                } else {
                    break;
                }
            }

            if(!continous) {
                m_ejectionTime -= dt;
            }
        }

        // Update particles
        for(auto &it : m_effect->modificators()) {
            it->updateParticle(m_particles, dt);
        }

        Matrix4 world(transform()->worldTransform());
        Vector3 cameraPos(camera->transform()->worldPosition());

        uint32_t visibleCount = 0;
        for(int i = 0; i < m_particles.size(); i++) {
            ParticleTransientData &p = m_particles[i];
            p.life -= dt;

            if(p.life > 0.0f) {
                GpuQuadParticle &q = m_quads[visibleCount];

                q.worldPosition = (local) ? world * p.position : p.position;

                q.sizeRot.x = p.size.x;
                q.sizeRot.y = p.size.y;
                q.sizeRot.z = p.rotation.z;

                q.uvScaleDist.x = p.uv.x;
                q.uvScaleDist.y = p.uv.y;
                q.uvScaleDist.z = (cameraPos - q.worldPosition).sqrLength();

                q.uvOffset.x = p.uv.z;
                q.uvOffset.y = p.uv.w;

                q.color = p.color;

                visibleCount++;
            }
        }

        std::sort(m_quads.begin(), m_quads.end(), [](const GpuQuadParticle &left, const GpuQuadParticle &right) { return left.uvScaleDist.z > right.uvScaleDist.z; });

        MaterialInstance *instance = m_materials.front();
        instance->setInstanceCount(visibleCount);

        memcpy(instance->rawUniformBuffer().data(), m_quads.data(), sizeof(GpuQuadParticle) * m_quads.size());
    }
}
/*!
    \internal
*/
Mesh *ParticleRender::meshToDraw() const {
    return m_effect ? m_effect->mesh() : nullptr;
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
AABBox ParticleRender::localBound() const {
    if(m_effect) {
        m_effect->bound();
    }
    return Renderable::localBound();
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

    std::string ref = Engine::reference(m_effect);
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

        // Update particles pool
        p->m_particles.resize(capacity);

        p->m_quads.resize(capacity);

        Vector4 colorID(CommandBuffer::idToColor(p->actor()->uuid()));

        for(int i = 0; i < capacity; i++) {
            p->m_quads[i].worldPosition.w = colorID.x;
            p->m_quads[i].sizeRot.w       = colorID.y;
            p->m_quads[i].uvScaleDist.w   = colorID.z;
            p->m_quads[i].uvOffset.w      = colorID.w;
        }
    }
}
