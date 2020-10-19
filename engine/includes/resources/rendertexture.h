#ifndef RENDERTEXTURE_H
#define RENDERTEXTURE_H

#include "texture.h"

class RenderTexturePrivate;

class NEXT_LIBRARY_EXPORT RenderTexture : public Texture {
    A_REGISTER(RenderTexture, Texture, Resources)

    A_PROPERTIES(
        A_PROPERTY(int, depth, RenderTexture::depth, RenderTexture::setDepth)
    )
    A_METHODS(
        A_METHOD(void, RenderTexture::setFixed)
    )
    A_NOENUMS()

public:
    RenderTexture ();
    ~RenderTexture ();

    void setTarget (FormatType format);

    int depth () const;
    void setDepth (int bits);

    void setFixed (bool fixed);

    void resize (int width, int height);

protected:
    virtual void makeCurrent (uint32_t index = 0) const;

private:
    RenderTexturePrivate *p_ptr;

};

#endif // RENDERTEXTURE_H
