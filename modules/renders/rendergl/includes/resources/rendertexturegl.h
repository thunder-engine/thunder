#ifndef RENDERTEXTUREVK_H
#define RENDERTEXTUREVK_H

#include <resources/rendertexture.h>

class RenderTextureGL : public RenderTexture {
    A_OVERRIDE(RenderTextureGL, RenderTexture, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    RenderTextureGL();

    uint32_t buffer() const { return m_Buffer; }

    void *nativeHandle() override;

    void makeCurrent(uint32_t index = 0) const override;

private:
    void updateTexture();
    void destroyTexture();

    uint32_t m_Buffer;

    uint32_t m_ID;

};

#endif // RENDERTEXTUREVK_H
