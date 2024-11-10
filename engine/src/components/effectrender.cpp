#include "effectrender.h"

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

enum class EmitterDefaultAttributes {
    Age = 0,
    DeltaTime,
    SpawnCounter,
    LastAttribute
};

enum class ParticleDefaultAttributes {
    Age = 0,
    Lifetime,
    Position,
    Rotation,
    Size,
    Color,
    Velocity,
    LastAttribute
};

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

        bool local = m_effect->local();
        bool continous = m_effect->continous();

        float &emitterAge = m_emitterData[static_cast<int32_t>(EmitterDefaultAttributes::Age)];
        float &deltaTime = m_emitterData[static_cast<int32_t>(EmitterDefaultAttributes::DeltaTime)];

        deltaTime = dt;

        if(continous || emitterAge > 0.0f) {
            float &emitterSpawnCounter = m_emitterData[static_cast<int32_t>(EmitterDefaultAttributes::SpawnCounter)];

            emitterSpawnCounter += m_effect->spawnRate() * deltaTime;

            if(emitterAge > 0.0f) {
                emitterAge -= dt;
            }
        }

        m_effect->update(m_emitterData, m_particleData);

        Matrix4 world(transform()->worldTransform());
        Vector3 cameraPos(camera->transform()->worldPosition());

        int capacity = m_effect->capacity();
        int stride = m_effect->attributeStride();

        uint32_t visibleCount = 0;
        for(int index = 0; index < capacity; index++) {
            int i = index * stride;

            if(m_particleData[i] > 0.0f) {
                GpuQuadParticle &q = m_quads[visibleCount];

                Vector3 position(m_particleData[i + 2], m_particleData[i + 3], m_particleData[i + 4]);
                q.worldPosition = (local) ? world * position : position;

                q.sizeRot.x = 1.0f; //p.size.x;
                q.sizeRot.y = 1.0f; //p.size.y;
                q.sizeRot.z = 0.0f; //p.rotation.z * DEG2RAD;

                q.uvScaleDist.x = 1.0f; //p.uv.x;
                q.uvScaleDist.y = 1.0f; //p.uv.y;
                q.uvScaleDist.z = cameraPos.dot(q.worldPosition);

                q.uvOffset.x = 0.0f; //p.uv.z;
                q.uvOffset.y = 0.0f; //p.uv.w;

                q.color.x = m_particleData[i + 11];
                q.color.y = m_particleData[i + 12];
                q.color.z = m_particleData[i + 13];
                q.color.w = m_particleData[i + 14];

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
Mesh *EffectRender::meshToDraw() const {
    return m_effect ? m_effect->mesh() : nullptr;
}
/*!
    Returns a ParticleEffect assigned to the this component.
*/
ParticleEffect *EffectRender::effect() const {
    return m_effect;
}
/*!
    Assgines a particle \a effect to the this component.
*/
void EffectRender::setEffect(ParticleEffect *effect) {
    PROFILE_FUNCTION();

    if(m_effect != effect) {
        if(m_effect) {
            m_effect->unsubscribe(this);
        }

        m_effect = effect;
        if(m_effect) {
            effectUpdated(Resource::Ready, this);
            m_effect->subscribe(&EffectRender::effectUpdated, this);
        }
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
        p->m_emitterData.resize(static_cast<int32_t>(EmitterDefaultAttributes::LastAttribute));
        // Update particles buffer
        p->m_particleData.resize(capacity * p->m_effect->attributeStride());

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
