#ifndef EFFECTRENDER_H
#define EFFECTRENDER_H

#include "renderable.h"

#include "resources/particleeffect.h"

struct GpuQuadParticle {
    // xyz - world position, w - objectID.x
    Vector4 worldPosition;

    // xy - size, z - rotation, w - objectID.y
    Vector4 sizeRot;

    // xy - uvScale, z - distance, w - objectID.z
    Vector4 uvScaleDist;

    // xy - uvOffset, z - unused, w - objectID.w
    Vector4 uvOffset;

    // xyzw - color
    Vector4 color;
};

class ENGINE_EXPORT EffectRender : public Renderable {
    A_REGISTER(EffectRender, Renderable, Components/Effects)

    A_PROPERTIES(
        A_PROPERTYEX(ParticleEffect *, effect, EffectRender::effect, EffectRender::setEffect, "editor=Asset")
    )
    A_NOMETHODS()

public:
    EffectRender();
    ~EffectRender() override;

    ParticleEffect *effect() const;
    void setEffect(ParticleEffect *effect);

    void deltaUpdate(float dt);

private:
    AABBox localBound() const override;

    Mesh *meshToDraw() const override;

    void update() override;

    static void effectUpdated(int state, void *ptr);

private:
    std::vector<float> m_emitterData;

    std::vector<float> m_particleData;

    std::vector<GpuQuadParticle> m_quads;

    std::vector<int32_t> m_offsets;

    ParticleEffect *m_effect;
};

#endif // EFFECTRENDER_H
