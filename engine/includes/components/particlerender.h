#ifndef PARTICLERENDER_H
#define PARTICLERENDER_H

#include "renderable.h"

class ParticleEffect;
class ParticleRenderPrivate;

class NEXT_LIBRARY_EXPORT ParticleRender : public Renderable {
    A_REGISTER(ParticleRender, Renderable, Components)

    A_PROPERTIES(
        A_PROPERTYEX(ParticleEffect *, Effect, ParticleRender::effect, ParticleRender::setEffect, "editor=Template")
    )
    A_NOMETHODS()

public:
    ParticleRender();
    ~ParticleRender();

    ParticleEffect *effect () const;
    void setEffect (ParticleEffect *effect);

private:
    void draw (ICommandBuffer &buffer, uint32_t layer) override;

    void update () override;

    void loadUserData (const VariantMap &data) override;
    VariantMap saveUserData () const override;

private:
    ParticleRenderPrivate *m_ptr;

};

#endif // PARTICLERENDER_H
