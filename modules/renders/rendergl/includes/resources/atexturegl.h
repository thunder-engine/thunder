#ifndef TEXTUREGL_H
#define TEXTUREGL_H

#include <deque>

#include <resources/texture.h>

#include "agl.h"

class ATextureGL : public Texture {
    A_OVERRIDE(ATextureGL, Texture, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    ATextureGL();

private:
    void *nativeHandle() override;
    void readPixels(int x, int y, int width, int height) override;

    void updateTexture();
    void destroyTexture();

    bool uploadTexture(const Sides *sides, uint32_t imageIndex, uint32_t target, uint32_t internal, uint32_t format, uint32_t index = 0);
    bool uploadTextureCubemap(const Sides *sides, uint32_t target, uint32_t internal, uint32_t format);

    inline bool isDwordAligned();
    inline int32_t dwordAlignedLineSize(int32_t width, int32_t bpp);

    uint32_t m_ID;
};

#endif // TEXTUREGL_H
