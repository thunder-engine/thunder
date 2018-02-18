#ifndef SPRITEGL
#define SPRITEGL

#include <components/sprite.h>

#include "apipeline.h"

class AShader;

class ASpriteGL : public Sprite, public IDrawObjectGL {
    A_OVERRIDE(ASpriteGL, Sprite, Components)

    A_NOMETHODS()
    A_NOPROPERTIES()

public:
    ASpriteGL                   ();

    void                        draw            (APipeline &pipeline, int8_t layer);

};

#endif // SPRITEGL

