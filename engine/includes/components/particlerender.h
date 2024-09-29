#ifndef PARTICLERENDER_H
#define PARTICLERENDER_H

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

class ENGINE_EXPORT ParticleRender : public Renderable {
    A_REGISTER(ParticleRender, Renderable, Components/Effects)

    A_PROPERTIES(
        A_PROPERTYEX(ParticleEffect *, effect, ParticleRender::effect, ParticleRender::setEffect, "editor=Asset")
    )
    A_NOMETHODS()

public:
    ParticleRender();
    ~ParticleRender() override;

    ParticleEffect *effect() const;
    void setEffect(ParticleEffect *effect);

    void deltaUpdate(float dt);

private:
    AABBox localBound() const override;

    Mesh *meshToDraw() const override;

    void update() override;

    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

    static void effectUpdated(int state, void *ptr);

private:
    std::vector<ParticleTransientData> m_particles;

    std::vector<GpuQuadParticle> m_quads;

    ParticleEffect *m_effect;

    float m_ejectionTime;

    float m_count;
};

#endif // PARTICLERENDER_H
