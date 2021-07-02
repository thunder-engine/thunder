#ifndef RENDERTEXTUREGL_H
#define RENDERTEXTUREGL_H

#include <resources/rendertexture.h>

class RenderTextureGL : public RenderTexture {
    A_OVERRIDE(RenderTextureGL, RenderTexture, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    RenderTextureGL();

    uint32_t buffer();

    uint32_t nativeHandle();

private:
    void updateTexture();
    void destroyTexture();

    uint32_t m_Buffer;

    uint32_t m_ID;

};

#endif // RENDERTEXTUREGL_H
