#ifndef TEXTURE_H
#define TEXTURE_H

#include <amath.h>

#include "resource.h"

class TexturePrivate;

class ENGINE_EXPORT Texture : public Resource {
    A_REGISTER(Texture, Resource, Resources)

    A_PROPERTIES(
        A_PROPERTY(int, width, Texture::width, Texture::setWidth),
        A_PROPERTY(int, height, Texture::height, Texture::setHeight),
        A_PROPERTY(int, format, Texture::format, Texture::setFormat),
        A_PROPERTY(int, wrap, Texture::wrap, Texture::setWrap),
        A_PROPERTY(int, filtering, Texture::filtering, Texture::setFiltering)
    )

    A_METHODS(
        A_METHOD(void, Texture::readPixels),
        A_METHOD(int,  Texture::getPixel),
        A_METHOD(bool, Texture::isCompressed),
        A_METHOD(bool, Texture::isCubemap),
        A_METHOD(void, Texture::setDirty),
        A_METHOD(void, Texture::resize)
    )

    A_ENUMS(
        A_ENUM(FormatType,
               A_VALUE(R8),
               A_VALUE(RGB8),
               A_VALUE(RGBA8),
               A_VALUE(R11G11B10Float),
               A_VALUE(Depth),
               A_VALUE(RGBA32Float)),

        A_ENUM(FilteringType,
               A_VALUE(None),
               A_VALUE(Bilinear),
               A_VALUE(Trilinear)),

        A_ENUM(WrapType,
               A_VALUE(Clamp),
               A_VALUE(Repeat),
               A_VALUE(Mirrored))
    )

public:
    enum FormatType {
        R8,
        RGB8,
        RGBA8,
        RGB10A2,
        R11G11B10Float,
        Depth,
        RGBA32Float
    };

    enum CompressionType {
        Uncompressed,
        DXT1,
        DXT5,
        ETC2
    };

    enum FilteringType {
        None,
        Bilinear,
        Trilinear
    };

    enum WrapType {
        Clamp,
        Repeat,
        Mirrored
    };

    typedef deque<ByteArray> Surface;
    typedef deque<Surface>   Sides;

public:
    Texture();
    ~Texture();

    virtual void readPixels(int x, int y, int width, int height);
    int getPixel(int x, int y, int level) const;
    ByteArray getPixels(int level) const;

    int width() const;
    void setWidth(int width);

    int height() const;
    void setHeight(int height);

    bool isFramebuffer() const;
    bool isCompressed() const;
    bool isCubemap() const;
    bool isArray() const;

    Surface &surface(int face);
    void addSurface(const Surface &surface);

    void setDirty();

    void resize(int width, int height);

    int format() const;
    void setFormat(int type);

    int wrap() const;
    void setWrap(int type);

    int filtering() const;
    void setFiltering(int type);

    int depthBits() const;
    void setDepthBits(int depth);

    void clear();

    static uint32_t maxTextureSize();
    static void setMaxTextureSize(uint32_t size);

    static uint32_t maxCubemapSize();
    static void setMaxCubemapSize(uint32_t size);

protected:
    int32_t m_format;
    int32_t m_compress;
    int32_t m_filtering;
    int32_t m_wrap;

    int32_t m_width;
    int32_t m_height;

    int32_t m_depth;

    Vector2Vector m_shape;
    Texture::Sides m_sides;

    static uint32_t s_maxTextureSize;
    static uint32_t s_maxCubemapSize;

protected:
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

    void switchState(ResourceState state) override;
    bool isUnloadable() override;

    Sides *getSides();

    int32_t size(int32_t width, int32_t height) const;
    int32_t sizeDXTc(int32_t width, int32_t height) const;
    int32_t sizeRGB(int32_t width, int32_t height) const;

    bool isDwordAligned();
    int32_t dwordAlignedLineSize(int32_t width, int32_t bpp);

    uint8_t components() const;

};

#endif // TEXTURE_H
