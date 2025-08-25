#ifndef SPRITEMODULE_H
#define SPRITEMODULE_H

#include "renderablemodule.h"
#include "effectrootnode.h"

class SpriteParticle : public RenderableModule {
    A_OBJECT(SpriteParticle, RenderableModule, Modificator)

    A_PROPERTIES(
        A_PROPERTY(Vector2, subUvSize, SpriteParticle::uvSize, SpriteParticle::setUvSize)
    )

public:
    SpriteParticle();

    void setRoot(EffectRootNode *effect) override;

    int type() const override { return Material::Billboard; }

    Vector2 uvSize() const;
    void setUvSize(const Vector2 &size);

private:
    void getOperations(std::vector<OperationData> &operations) const override;

protected:
    Vector2 m_uvSize;

};

#endif // SPRITEPARTICLE_H
