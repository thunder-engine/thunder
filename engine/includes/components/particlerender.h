#ifndef PARTICLERENDER_H
#define PARTICLERENDER_H

#include "renderable.h"

#include "resources/particleeffect.h"

class MaterialInstance;

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

    static void effectUpdated(int state, void *ptr);

private:
    typedef vector<Matrix4> BufferArray;
    typedef list<ParticleData> ParticleList;

    vector<BufferArray> m_buffers;
    vector<ParticleList> m_particles;

    vector<MaterialInstance *> m_materials;

    vector<float> m_ejectionTime;
    vector<float> m_count;

    vector<uint32_t> m_visibleCount;

    AABBox m_aabb;

    ParticleEffect *m_effect;

};

#endif // PARTICLERENDER_H
