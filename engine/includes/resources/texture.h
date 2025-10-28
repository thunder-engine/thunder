#ifndef TEXTURE_H
#define TEXTURE_H

#include <amath.h>

#include "resource.h"

class CommandBuffer;

class ENGINE_EXPORT Texture : public Resource {
    A_OBJECT(Texture, Resource, Resources)

    A_PROPERTIES(
        A_PROPERTY(int, width, Texture::width, Texture::setWidth),
        A_PROPERTY(int, height, Texture::height, Texture::setHeight),
        A_PROPERTY(int, depth, Texture::depth, Texture::setDepth),
        A_PROPERTY(int, format, Texture::format, Texture::setFormat),
        A_PROPERTY(int, compress, Texture::compress, Texture::setCompress),
        A_PROPERTY(int, wrap, Texture::wrap, Texture::setWrap),
        A_PROPERTY(int, filtering, Texture::filtering, Texture::setFiltering)
    )

    A_METHODS(
        A_METHOD(int,  Texture::getPixel),
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
        RGBA32Float,
        RGBA16Float,
        Depth
    };

    enum CompressionType {
        Uncompressed,
        BC1,
        BC3,
        BC7,
        ASTC,
        ETC1,
        ETC2,
        PVRTC
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

    enum Flags {
        Render   = (1<<0),
        Feedback = (1<<1)
    };

    typedef std::deque<ByteArray> Surface;
    typedef std::deque<Surface> Sides;

public:
    Texture();
    ~Texture();

    int width() const;
    void setWidth(int width);

    int height() const;
    void setHeight(int height);

    int depth() const;
    void setDepth(int depth);

    int flags() const;
    void setFlags(int flags);

    bool isRender() const;
    bool isFeedback() const;
    bool isCubemap() const;
    bool isArray() const;

    int sides() const;
    int mipCount() const;

    Surface &surface(int side);
    void addSurface(const Surface &surface);

    void setDirty();

    int format() const;
    void setFormat(int type);

    int compress() const;
    void setCompress(int method);

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

    virtual void readPixels(int x, int y, int width, int height);
    int getPixel(int x, int y, int level) const;
    ByteArray getPixels(int level) const;

    void resize(int width, int height);

protected:
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

    void switchState(Resource::State state) override;
    bool isUnloadable() override;

    int32_t sizeRGB(int32_t width, int32_t height, int32_t depth) const;

    bool isDwordAligned();
    int32_t dwordAlignedLineSize(int32_t width, int32_t bpp);

    uint8_t components() const;

protected:
    Texture::Sides m_sides;

    int32_t m_format;
    int32_t m_compress;
    int32_t m_filtering;
    int32_t m_wrap;

    int32_t m_width;
    int32_t m_height;
    int32_t m_depth;

    int32_t m_depthBits;

    int32_t m_flags;

    static uint32_t s_maxTextureSize;
    static uint32_t s_maxCubemapSize;

};

#endif // TEXTURE_H
