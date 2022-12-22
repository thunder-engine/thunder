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

typedef vector<Matrix4> BufferArray;
typedef list<ParticleData> ParticleList;

struct EmitterRender {
    EmitterRender() :
            m_material(nullptr),
            m_ejectionTime(0.0f),
            m_count(0.0f),
            m_visibleCount(0) {

        m_particles.resize(1);
        m_buffer.resize(1);
    }

    ~EmitterRender() {
        delete m_material;
    }

    BufferArray m_buffer;
    ParticleList m_particles;

    MaterialInstance *m_material;

    float m_ejectionTime;
    float m_count;

    uint32_t m_visibleCount;
};
typedef deque<EmitterRender> EmitterArray;

class ParticleRenderPrivate : public Resource::IObserver {
public:
    ParticleRenderPrivate() :
            m_effect(nullptr) {

    }

    ~ParticleRenderPrivate() {
        if(m_effect) {
            m_effect->unsubscribe(this);
        }
    }

    void resourceUpdated(const Resource *resource, Resource::ResourceState state) override {
        if(resource == m_effect && state == Resource::Ready) {
            m_emitters.clear();
            m_emitters.resize(m_effect->emittersCount());

            for(int32_t i = 0; i < m_effect->emittersCount(); i++) {
                ParticleEmitter *emitter = m_effect->emitter(i);
                if(emitter->material()) {
                    m_emitters[i].m_material = emitter->material()->createInstance(Material::Billboard);
                }
            }
        }
    }

    AABBox m_aabb;
    EmitterArray m_emitters;
    ParticleEffect *m_effect;

};
/*!
    \class ParticleRender
    \brief Draws a particle effect on the scene.
    \inmodule Engine

    The ParticleRender component allows you to display Particle Effects such as fire and explosions.
*/

ParticleRender::ParticleRender() :
        p_ptr(new ParticleRenderPrivate) {

}

ParticleRender::~ParticleRender() {
    delete p_ptr;
    p_ptr = nullptr;
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

    if(p_ptr->m_effect) {
        p_ptr->m_aabb.setBox(Vector3(FLT_MAX), Vector3(-FLT_MAX));

        uint32_t index  = 0;
        for(auto &it : p_ptr->m_emitters) {
            ParticleEmitter *emitter = p_ptr->m_effect->emitter(index);

            it.m_visibleCount = 0;
            uint32_t i = 0;
            bool local = emitter->local();
            bool continous = emitter->continous();

            // Spawn particles
            it.m_count += emitter->distibution() * dt;
            while(isEnabled() && (continous || it.m_ejectionTime > 0.0f) && it.m_count >= 1.0f) {
                ParticleData particle;
                spawnParticle(*emitter, particle);
                particle.transform = m * particle.position;
                it.m_particles.push_back(particle);
                it.m_count -= 1.0f;
            }

            if(it.m_buffer.size() < it.m_particles.size()) {
                it.m_buffer.resize(it.m_particles.size());
            }

            if(!continous) {
                it.m_ejectionTime -= dt;
            }
            // Update particles
            for(auto &particle : it.m_particles) {
                particle.life -= dt;
                if(particle.life >= 0.0f) {
                    updateParticle(*emitter, particle, dt);

                    particle.transform = (local) ? m * particle.position : particle.position;
                    particle.distance = (pos - particle.transform).sqrLength();

                    it.m_visibleCount++;
                } else {
                    particle.distance = -1.0f;
                    if(isEnabled() && (continous || it.m_ejectionTime > 0.0f) && it.m_count >= 1.0f) {
                        spawnParticle(*emitter, particle);
                        particle.transform = m * particle.position;
                        it.m_count -= 1.0f;
                    }
                }
                it.m_buffer[i].mat[0]  = particle.transform.x;
                it.m_buffer[i].mat[1]  = particle.transform.y;
                it.m_buffer[i].mat[2]  = particle.transform.z;

                it.m_buffer[i].mat[3]  = particle.angle.z;

                it.m_buffer[i].mat[4]  = particle.size.x;
                it.m_buffer[i].mat[5]  = particle.size.y;
                it.m_buffer[i].mat[6]  = particle.size.z;

                it.m_buffer[i].mat[7]  = particle.distance;

                it.m_buffer[i].mat[10] = particle.frame;
                it.m_buffer[i].mat[11] = particle.life;

                it.m_buffer[i].mat[12] = particle.color.x;
                it.m_buffer[i].mat[13] = particle.color.y;
                it.m_buffer[i].mat[14] = particle.color.z;
                it.m_buffer[i].mat[15] = particle.color.w;

                if(particle.life > 0.0f) {
                    p_ptr->m_aabb.encapsulate(particle.transform, particle.size.sqrLength());
                }

                i++;
            }

            std::sort(it.m_buffer.begin(), it.m_buffer.end(), [](const Matrix4 &left, const Matrix4 &right) { return left[7] > right[7]; });

            index++;
        }
    }
}
/*!
    \internal
*/
void ParticleRender::draw(CommandBuffer &buffer, uint32_t layer) {
    Actor *a = actor();
    if(layer & a->layers()) {
        if(layer & CommandBuffer::RAYCAST) {
            buffer.setColor(CommandBuffer::idToColor(a->uuid()));
        }
        uint32_t index = 0;
        for(auto &it : p_ptr->m_emitters) {
            if(it.m_visibleCount > 0) {
                ParticleEmitter *emitter = p_ptr->m_effect->emitter(index);
                buffer.drawMeshInstanced(&it.m_buffer[0], it.m_visibleCount, emitter->mesh(), 0, layer, it.m_material);
            }
            index++;
        }
        buffer.setColor(Vector4(1.0f));
    }
}
/*!
    Returns a ParticleEffect assigned to the this component.
*/
ParticleEffect *ParticleRender::effect() const {
    return p_ptr->m_effect;
}
/*!
    Assgines a particle \a effect to the this component.
*/
void ParticleRender::setEffect(ParticleEffect *effect) {
    if(effect) {
        if(p_ptr->m_effect) {
            p_ptr->m_effect->unsubscribe(p_ptr);
        }
        p_ptr->m_effect = effect;
        p_ptr->resourceUpdated(effect, Resource::Ready);
        p_ptr->m_effect->subscribe(p_ptr);
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
    return p_ptr->m_aabb;
}
/*!
    \internal
*/
void ParticleRender::loadUserData(const VariantMap &data) {
    Component::loadUserData(data);
    {
        auto it = data.find(EFFECT);
        if(it != data.end()) {
            setEffect(Engine::loadResource<ParticleEffect>((*it).second.toString()));
        }
    }
}
/*!
    \internal
*/
VariantMap ParticleRender::saveUserData() const {
    VariantMap result = Component::saveUserData();
    {
        string ref = Engine::reference(p_ptr->m_effect);
        if(!ref.empty()) {
            result[EFFECT] = ref;
        }
    }
    return result;
}
