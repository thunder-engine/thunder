#ifndef EFFECTRENDER_H
#define EFFECTRENDER_H

#include "renderable.h"

#include "resources/visualeffect.h"

class ENGINE_EXPORT EffectRender : public Renderable {
    A_OBJECT(EffectRender, Renderable, Components/Effects)

    A_PROPERTIES(
        A_PROPERTYEX(VisualEffect *, effect, EffectRender::effect, EffectRender::setEffect, "editor=Asset")
    )
    A_NOMETHODS()

public:
    EffectRender();
    ~EffectRender() override;

    VisualEffect *effect() const;
    void setEffect(VisualEffect *effect);

    void deltaUpdate(float dt);

private:
    AABBox localBound() const override;

    Mesh *meshToDraw(int instance) const override;

    void update() override;

    static void effectUpdated(int state, void *ptr);

private:
    std::vector<float> m_emitterData;

    std::vector<float> m_particleData;

    std::vector<float> m_renderData;

    std::vector<int32_t> m_offsets;

    VisualEffect *m_effect;
};

#endif // EFFECTRENDER_H
