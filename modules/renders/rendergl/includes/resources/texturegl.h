#ifndef TEXTUREGL_H
#define TEXTUREGL_H

#include <resources/texture.h>

class TextureGL : public Texture {
    A_OVERRIDE(TextureGL, Texture, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    TextureGL();

    uint32_t nativeHandle();

private:
    void readPixels(int x, int y, int width, int height) override;

    void updateTexture();
    void destroyTexture();

    bool uploadTexture(const Sides *sides, uint32_t imageIndex, uint32_t target, uint32_t internal, uint32_t format, uint32_t type);
    bool uploadTextureCubemap(const Sides *sides, uint32_t target, uint32_t internal, uint32_t format, uint32_t type);

    uint32_t m_id;

};

#endif // TEXTUREGL_H
