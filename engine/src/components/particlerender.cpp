#include "particlerender.h"

#include <algorithm>

#include "particleeffect.h"

#include "timer.h"

#include "actor.h"
#include "transform.h"
#include "camera.h"

#include "commandbuffer.h"

#define EFFECT "Effect"

typedef vector<Matrix4> BufferArray;
typedef list<ParticleEffect::ParticleData> ParticleList;

bool comp(const Matrix4 &left, const Matrix4 &right) {
    return left[7] > right[7];
}

struct Emitter {
    Emitter() :
            m_Countdown(0.0f),
            m_Counter(0.0f),
            m_Count(0) {

        m_Particles.resize(1);
        m_Buffer.resize(1);
    }

    BufferArray m_Buffer;
    ParticleList m_Particles;

    float m_Countdown;
    float m_Counter;

    uint32_t m_Count;
};
typedef deque<Emitter> EmitterArray;

class ParticleRenderPrivate {
public:
    ParticleRenderPrivate() :
            m_pEffect(nullptr) {

    }

    ParticleEffect *m_pEffect;
    EmitterArray m_Emitters;
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
}
/*!
    \internal
*/
void ParticleRender::update() {
    float dt    = Timer::deltaTime() * Timer::scale();
    Matrix4 &m  = actor()->transform()->worldTransform();

    Camera *camera = Camera::current();
    if(!camera) {
        return;
    }
    Vector3 pos = camera->actor()->transform()->worldPosition();

    if(p_ptr->m_pEffect) {
        uint32_t index  = 0;
        for(auto &it : p_ptr->m_Emitters) {
            ParticleEffect::Emitter &emitter = p_ptr->m_pEffect->emitter(index);
            it.m_Count = 0;
            uint32_t i = 0;
            for(auto &particle : it.m_Particles) {
                particle.life   -= dt;
                if(particle.life >= 0.0f) {
                    p_ptr->m_pEffect->updateParticle(index, particle, dt);

                    particle.transform  = (emitter.m_Local) ? m * particle.position : particle.position;
                    particle.distance   = (pos - particle.transform).sqrLength();

                    it.m_Count++;
                } else {
                    particle.distance   = -1.0f;
                    if(isEnabled() && (emitter.m_Continous || it.m_Countdown > 0.0f) && it.m_Counter >= 1.0f) {
                        p_ptr->m_pEffect->spawnParticle(index, particle);
                        particle.transform  = m * particle.position;
                        it.m_Counter   -= 1.0f;
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

                i++;
            }

            while(isEnabled() && (emitter.m_Continous || it.m_Countdown > 0.0f) && it.m_Counter >= 1.0f) {
                ParticleEffect::ParticleData particle;
                p_ptr->m_pEffect->spawnParticle(index, particle);
                particle.transform  = m * particle.position;
                it.m_Particles.push_back(particle);
                it.m_Counter   -= 1.0f;
            }

            if(it.m_Buffer.size() < it.m_Particles.size()) {
                it.m_Buffer.resize(it.m_Particles.size());
            }

            it.m_Counter   += emitter.m_Distibution * dt;
            if(!emitter.m_Continous) {
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
void ParticleRender::draw(ICommandBuffer &buffer, uint32_t layer) {
    Actor *a = actor();
    if(layer & a->layers()) {
        if(layer & ICommandBuffer::RAYCAST) {
            buffer.setColor(ICommandBuffer::idToColor(a->uuid()));
        }
        uint32_t index  = 0;
        for(auto &it : p_ptr->m_Emitters) {
            if(it.m_Count > 0) {
                ParticleEffect::Emitter &emitter = p_ptr->m_pEffect->emitter(index);
                buffer.drawMeshInstanced(&it.m_Buffer[0], it.m_Count, emitter.m_pMesh, layer, emitter.m_pMaterial);
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
        p_ptr->m_pEffect   = effect;
        p_ptr->m_Emitters.clear();
        p_ptr->m_Emitters.resize(effect->emittersCount());
    }
}
/*!
    \internal
*/
AABBox ParticleRender::bound() const {
    if(p_ptr->m_pEffect) {
        return p_ptr->m_pEffect->bound() * actor()->transform()->worldTransform();
    }
    return Renderable::bound();
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
