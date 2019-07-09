#ifndef RENDERTEXTURE_H
#define RENDERTEXTURE_H

#include "texture.h"

class RenderTexturePrivate;

class NEXT_LIBRARY_EXPORT RenderTexture : public Texture {
    A_REGISTER(RenderTexture, Texture, Resources)

public:
    RenderTexture ();
    ~RenderTexture ();

    void setTarget (FormatType format);

    uint8_t depth () const;
    void setDepth (uint8_t bits);

    void setFixed (bool fixed);

    void resize (int32_t width, int32_t height);

    virtual void makeCurrent (uint32_t index = 0) const;

private:
    RenderTexturePrivate *p_ptr;

};

#endif // RENDERTEXTURE_H
