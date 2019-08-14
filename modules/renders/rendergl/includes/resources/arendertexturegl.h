#ifndef ARENDERTEXTUREGL_H
#define ARENDERTEXTUREGL_H

#include <resources/rendertexture.h>

#include "agl.h"

class ARenderTextureGL : public RenderTexture {
    A_OVERRIDE(ARenderTextureGL, RenderTexture, Resources)

public:
    ARenderTextureGL            ();

    uint32_t                    buffer                      () const { return m_Buffer; }

    void                       *nativeHandle                () override;

    void                        makeCurrent                 (uint32_t index = 0) const override;

private:
    void                        updateTexture               ();
    void                        destroyTexture              ();

    uint32_t                    m_Buffer;

    uint32_t                    m_ID;

};

#endif // ARENDERTEXTUREGL_H
