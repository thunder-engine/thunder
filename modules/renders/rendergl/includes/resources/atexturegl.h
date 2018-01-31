#ifndef TEXTUREGL_H
#define TEXTUREGL_H

#include <deque>

#include <resources/texture.h>

#include "agl.h"

typedef deque<uint8_t *>    Surface;
typedef deque<Surface>      Sides;

class ATextureGL : public Texture {
    A_OVERRIDE(ATextureGL, Texture, Resources)
public:
    ATextureGL                  ();

    void                        create                      (uint32_t target, uint32_t internal, uint32_t format, uint32_t bits);

    void                        resize                      (uint32_t width, uint32_t height);

    void                        loadUserData                (const AVariantMap &data);

    void                        bind                        () const;
    void                        unbind                      () const;

    uint32_t                    id                          () const        { return mID; }

private:
    void                        clear                       ();

    uint32_t                    size                        (uint32_t width, uint32_t height) const;
    uint32_t                    sizeDXTc                    (uint32_t width, uint32_t height) const;
    uint32_t                    sizeRGB                     (uint32_t width, uint32_t height) const;

    bool                        uploadTexture2D             (const Sides &sides, uint32_t imageIndex = 0, uint32_t target = GL_TEXTURE_2D);
    bool                        uploadTextureCubemap        (const Sides &sides);

    inline bool                 isDwordAligned              ();
    inline uint32_t             dwordAlignedLineSize        (uint32_t width, uint32_t bpp) ;

    uint32_t                    m_Target;
    uint32_t                    mInternal;
    uint32_t                    m_Format;
    uint32_t                    m_Bits;

    uint32_t                    mID;

};

#endif // TEXTUREGL_H
