#ifndef SPRITEGL
#define SPRITEGL

#include <components/sprite.h>

#include "apipeline.h"

class ASpriteGL : public Sprite, public IDrawObject {
    A_OVERRIDE(ASpriteGL, Sprite, Components)

    A_NOMETHODS()
    A_NOPROPERTIES()

public:
    ASpriteGL                   ();

    void                        draw            (ICommandBuffer &buffer, int8_t layer);

protected:
    Mesh                       *m_pPlane;

};

#endif // SPRITEGL

