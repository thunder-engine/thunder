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

bool comp(const Matrix4 &left, const Matrix4 &right) {
    return left[7] > right[7];
}

struct EmitterRender {
    EmitterRender() :
            m_pInstance(nullptr),
            m_Countdown(0.0f),
            m_Counter(0.0f),
            m_Count(0) {

        m_Particles.resize(1);
        m_Buffer.resize(1);
    }

    ~EmitterRender() {
        delete m_pInstance;
    }

    BufferArray m_Buffer;
    ParticleList m_Particles;

    MaterialInstance *m_pInstance;

    float m_Countdown;
    float m_Counter;

    uint32_t m_Count;
};
typedef deque<EmitterRender> EmitterArray;

class ParticleRenderPrivate : public Resource::IObserver {
public:
    ParticleRenderPrivate() :
            m_pEffect(nullptr) {

    }

    ~ParticleRenderPrivate() {
        if(m_pEffect) {
            m_pEffect->unsubscribe(this);
        }
    }

    void resourceUpdated(const Resource *resource, Resource::ResourceState state) override {
        if(resource == m_pEffect && state == Resource::Ready) {
            m_Emitters.clear();
            m_Emitters.resize(m_pEffect->emittersCount());

            for(int32_t i = 0; i < m_pEffect->emittersCount(); i++) {
                ParticleEmitter *emitter = m_pEffect->emitter(i);
                if(emitter->material()) {
                    m_Emitters[i].m_pInstance = emitter->material()->createInstance(Material::Billboard);
                }
            }
        }
    }

    AABBox m_AABB;
    EmitterArray m_Emitters;
    ParticleEffect *m_pEffect;

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
    Matrix4 m(actor()->transform()->worldTransform());

    Camera *camera = Camera::current();
    if(!camera) {
        return;
    }
    Vector3 pos(camera->actor()->transform()->worldPosition());

    if(p_ptr->m_pEffect) {
        p_ptr->m_AABB.setBox(Vector3(FLT_MAX), Vector3(-FLT_MAX));

        uint32_t index  = 0;
        for(auto &it : p_ptr->m_Emitters) {
            ParticleEmitter *emitter = p_ptr->m_pEffect->emitter(index);
            it.m_Count = 0;
            uint32_t i = 0;
            bool local = emitter->local();
            bool continous = emitter->continous();
            for(auto &particle : it.m_Particles) {
                particle.life -= dt;
                if(particle.life >= 0.0f) {
                    updateParticle(*emitter, particle, dt);

                    particle.transform = (local) ? m * particle.position : particle.position;
                    particle.distance = (pos - particle.transform).sqrLength();

                    it.m_Count++;
                } else {
                    particle.distance = -1.0f;
                    if(isEnabled() && (continous || it.m_Countdown > 0.0f) && it.m_Counter >= 1.0f) {
                        spawnParticle(*emitter, particle);
                        particle.transform  = m * particle.position;
                        it.m_Counter -= 1.0f;
                    }
                }
                it.m_Buffer[i].mat[0]  = particle.transform.x;
                it.m_Buffer[i].mat[1]  = particle.transform.y;
                it.m_Buffer[i].mat[2]  = particle.transform.z;

                it.m_Buffer[i].mat[3]  = particle.angle.z;

                it.m_Buffer[i].mat[4]  = particle.size.x;
                it.m_Buffer[i].mat[5]  = particle.size.y;
                it.m_Buffer[i].mat[6]  = particle.size.z;

                it.m_Buffer[i].mat[7]  = particle.distance;

                it.m_Buffer[i].mat[10] = particle.frame;
                it.m_Buffer[i].mat[11] = particle.life;

                it.m_Buffer[i].mat[12] = particle.color.x;
                it.m_Buffer[i].mat[13] = particle.color.y;
                it.m_Buffer[i].mat[14] = particle.color.z;
                it.m_Buffer[i].mat[15] = particle.color.w;

                if(particle.life > 0.0f) {
                    p_ptr->m_AABB.encapsulate(particle.transform, particle.size.sqrLength());
                }

                i++;
            }

            while(isEnabled() && (continous || it.m_Countdown > 0.0f) && it.m_Counter >= 1.0f) {
                ParticleData particle;
                spawnParticle(*emitter, particle);
                particle.transform  = m * particle.position;
                it.m_Particles.push_back(particle);
                it.m_Counter -= 1.0f;
            }

            if(it.m_Buffer.size() < it.m_Particles.size()) {
                it.m_Buffer.resize(it.m_Particles.size());
            }

            it.m_Counter += emitter->distibution() * dt;
            if(!continous) {
                it.m_Countdown -= dt;
            }

            std::sort(it.m_Buffer.begin(), it.m_Buffer.end(), comp);

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
        for(auto &it : p_ptr->m_Emitters) {
            if(it.m_Count > 0) {
                ParticleEmitter *emitter = p_ptr->m_pEffect->emitter(index);
                buffer.drawMeshInstanced(&it.m_Buffer[0], it.m_Count, emitter->mesh(), 0, layer, it.m_pInstance);
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
    return p_ptr->m_pEffect;
}
/*!
    Assgines a particle \a effect to the this component.
*/
void ParticleRender::setEffect(ParticleEffect *effect) {
    if(effect) {
        if(p_ptr->m_pEffect) {
            p_ptr->m_pEffect->unsubscribe(p_ptr);
        }
        p_ptr->m_pEffect = effect;
        p_ptr->resourceUpdated(effect, Resource::Ready);
        p_ptr->m_pEffect->subscribe(p_ptr);
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
    return p_ptr->m_AABB;
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
        string ref = Engine::reference(p_ptr->m_pEffect);
        if(!ref.empty()) {
            result[EFFECT] = ref;
        }
    }
    return result;
}
