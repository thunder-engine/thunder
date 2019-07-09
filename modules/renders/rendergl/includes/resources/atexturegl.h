#ifndef TEXTUREGL_H
#define TEXTUREGL_H

#include <deque>

#include <resources/texture.h>

#include "agl.h"

class ATextureGL : public Texture {
    A_OVERRIDE(ATextureGL, Texture, Resources)
public:
    ATextureGL                  ();

    void                       *nativeHandle                () override;

    void                        readPixels                  (int32_t x, int32_t y, int32_t width, int32_t height) override;

private:
    void                        updateTexture               ();
    void                        destroyTexture              ();

    bool                        uploadTexture2D             (const Sides *sides, uint32_t imageIndex, uint32_t target, uint32_t internal, uint32_t format);
    bool                        uploadTextureCubemap        (const Sides *sides, uint32_t internal, uint32_t format);

    inline bool                 isDwordAligned              ();
    inline int32_t              dwordAlignedLineSize        (int32_t width, int32_t bpp);

    uint32_t                    m_ID;
};

#endif // TEXTUREGL_H
