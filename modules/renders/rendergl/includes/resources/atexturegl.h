#ifndef TEXTUREGL_H
#define TEXTUREGL_H

#include <deque>

#include <resources/texture.h>

#include "agl.h"

class ATextureGL : public Texture {
    A_OVERRIDE(ATextureGL, Texture, Resources)
public:
    ATextureGL                  ();

    ~ATextureGL                 ();

    void                        apply                       ();

    void                        create                      (uint32_t target, uint32_t internal, uint32_t format, uint32_t bits);

    void                        destroy                     ();

    uint32_t                    buffer                      () const    { return m_Buffer; }

    void                        resize                      (uint32_t width, uint32_t height);

    void                        bind                        () const;
    void                        unbind                      () const;

    uint32_t                    id                          () const    { return mID; }

private:
    void                        clear                       ();

    bool                        uploadTexture2D             (const Sides &sides, uint32_t imageIndex = 0, uint32_t target = GL_TEXTURE_2D, bool update = false);
    bool                        uploadTextureCubemap        (const Sides &sides);

    inline bool                 isDwordAligned              ();
    inline uint32_t             dwordAlignedLineSize        (uint32_t width, uint32_t bpp) ;

    uint32_t                    m_Target;
    uint32_t                    m_Internal;
    uint32_t                    m_GlFormat;
    uint32_t                    m_Bits;

    uint32_t                    m_Buffer;

    uint32_t                    mID;
};

#endif // TEXTUREGL_H
