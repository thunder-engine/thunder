#ifndef TEXTUREMT_H
#define TEXTUREMT_H

#include <resources/texture.h>

#include "wrappermt.h"

class TextureMt : public Texture {
    A_OBJECT_OVERRIDE(TextureMt, Texture, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    TextureMt();

    MTL::Texture *nativeHandle();

    MTL::SamplerState *sampler();

    MTL::PixelFormat pixelFormat();

private:
    void readPixels(int x, int y, int width, int height) override;

    void updateTexture();

    void uploadTexture(uint32_t slice);

private:
    MTL::Texture *m_native;

    MTL::SamplerState *m_sampler;

    MTL::Buffer *m_buffer;

};

#endif // TEXTUREMT_H
