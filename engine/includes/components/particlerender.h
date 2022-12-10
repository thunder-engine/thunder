#ifndef PARTICLERENDER_H
#define PARTICLERENDER_H

#include "renderable.h"

class ParticleEmitter;
class ParticleData;
class ParticleEffect;
class ParticleRenderPrivate;

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
    void spawnParticle(ParticleEmitter &emitter, ParticleData &data);
    void updateParticle(ParticleEmitter &emitter, ParticleData &data, float dt);

    AABBox bound() const override;

    void draw(CommandBuffer &buffer, uint32_t layer) override;

    void update() override;

    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

private:
    ParticleRenderPrivate *p_ptr;

};

#endif // PARTICLERENDER_H
