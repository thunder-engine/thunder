#include "resources/texture.h"

#include <variant.h>

#include <cstring>

#define HEADER  "Header"
#define DATA    "Data"

class TexturePrivate {
public:
    TexturePrivate() :
            m_Format(Texture::R8),
            m_Compress(Texture::Uncompressed),
            m_Type(Texture::Flat),
            m_Filtering(Texture::None),
            m_Wrap(Texture::Clamp) {

    }

    Texture::FormatType m_Format;
    Texture::CompressionType m_Compress;
    Texture::TextureType m_Type;
    Texture::FilteringType m_Filtering;
    Texture::WrapType m_Wrap;

    int32_t m_Width;
    int32_t m_Height;

    Vector2Vector m_Shape;
    Texture::Sides m_Sides;
};

/*!
    \class Texture
    \brief Texture resource contains all necessary texture data.
    \inmodule Resource

    This class can be used to handle texture resource or create them at runtime.
*/

/*!
    \enum Texture::WrapType

    Wrap mode for textures.

    \value Clamp \c Clamps the texture to the last pixel at the edge.
    \value Repeat \c Tiles the texture, creating a repeating pattern.
    \value Mirrored \c Tiles the texture, creating a repeating pattern by mirroring it at every integer boundary.
*/

/*!
    \enum Texture::FormatType

    \value R8 \c Single channel(Red) texture. 8-bit integer
    \value RGB8 \c Color texture format. 8 bit integer per channel. 24-bits in total.
    \value RGBA8 \c Color texture format with alpha channel. 8-bit integer per channel. 32-bits in total.
    \value RGB10A2 \c 10 bits each for RGB, 2 for Alpha.
    \value RGB16Float \c Color texture with floating-point values. It uses 16-bit floating-point values per channel.
    \value RGBA32Float \c Color texture and alpha with floating-point values. It uses 32-bit floating-point values per channel.
    \value R11G11B10Float \c This uses special 11 and 10-bit floating-point values. This is very economical for floating-point values (using only 32-bits per value).
    \value Depth \c Depth buffer texture format. Number bits per pixel depend on graphical settings and hardware. Can be 16, 24 or 32-bit per pixel.
*/

/*!
    \enum Texture::FilteringType

    \value None \c Texture samples draw as is.
    \value Bilinear \c Texture samples are averaged.
    \value Trilinear \c Texture samples are averaged and also interpolated from adjacent mipmap levels.
*/

Texture::Texture() :
        p_ptr(new TexturePrivate) {

    setShape({ Vector2(0.0f), Vector2(0.0f, 1.0f), Vector2(1.0f), Vector2(1.0f, 0.0f) });
}

Texture::~Texture() {
    clear();

    delete p_ptr;
}
/*!
    \internal
*/
void Texture::loadUserData(const VariantMap &data) {
    clear();

    {
        auto it = data.find(HEADER);
        if(it != data.end()) {
            VariantList header = (*it).second.value<VariantList>();

            auto i = header.begin();
            p_ptr->m_Width = (*i).toInt();
            i++;
            p_ptr->m_Height = (*i).toInt();
            i++;
            //Reserved
            i++;

            p_ptr->m_Type = TextureType((*i).toInt());
            i++;
            p_ptr->m_Compress = CompressionType((*i).toInt());
            i++;
            p_ptr->m_Format = FormatType((*i).toInt());
            i++;
            p_ptr->m_Filtering = FilteringType((*i).toInt());
            i++;
            p_ptr->m_Wrap = WrapType((*i).toInt());
            i++;
        }
    }
    {
        auto it = data.find(DATA);
        if(it != data.end()) {
            const VariantList &surfaces = (*it).second.value<VariantList>();
            for(auto s : surfaces) {
                Surface img;
                int32_t w = p_ptr->m_Width;
                int32_t h = p_ptr->m_Height;
                const VariantList &lods = s.value<VariantList>();
                for(auto l : lods) {
                    ByteArray bits = l.toByteArray();
                    uint32_t s = size(w, h);
                    if(s && !bits.empty()) {
                        uint8_t *pixels = new uint8_t[s];
                        memcpy(pixels, &bits[0], s);
                        img.push_back(pixels);
                    }
                    w = MAX(w / 2, 1);
                    h = MAX(h / 2, 1);
                }
                p_ptr->m_Sides.push_back(img);
            }
        }
    }

    setState(ToBeUpdated);
}
/*!
    Returns a surface for the provided \a face.
    Each texture must contain at least one surface.
    Commonly used to set surfaces for the cube maps.
*/
Texture::Surface &Texture::surface(int face) {
    return p_ptr->m_Sides[face];
}
/*!
    Adds \a surface to the texture.
    Each texture must contain at least one surface.
    Commonly used to set surfaces for the cube maps.
*/
void Texture::addSurface(const Surface &surface) {
    p_ptr->m_Sides.push_back(surface);
}
/*!
    Marks texture as dirty.
    That means this texture must be forcefully reloaded.
*/
void Texture::setDirty() {
    setState(ToBeUpdated);
}
/*!
    \internal
*/
void Texture::clear() {
    p_ptr->m_Width = 1;
    p_ptr->m_Height = 1;

    for(auto side : p_ptr->m_Sides) {
        for(auto lod : side) {
            delete []lod;
        }
    }
    p_ptr->m_Sides.clear();
    p_ptr->m_Shape.clear();
}
/*!
    \internal
    Returns a native (underlying graphics API) pointer to the texture resource.
*/
void *Texture::nativeHandle() {
    return nullptr;
}
/*!
    Read pixels from GPU into texture data.
*/
void Texture::readPixels(int x, int y, int width, int height) {
    A_UNUSED(x);
    A_UNUSED(y);
    A_UNUSED(width);
    A_UNUSED(height);
}
/*!
    Returns pixel color as RGBA integer for example 0x00ff00ff which can be mapped to (0, 255, 0, 255)
*/
int Texture::getPixel(int x, int y) const {
    uint32_t result = 0;
    if(!p_ptr->m_Sides.empty() && !p_ptr->m_Sides[0].empty()) {
        uint8_t *ptr = p_ptr->m_Sides[0][0] + (y * p_ptr->m_Width + x);
        memcpy(&result, ptr, sizeof(uint32_t));
    }
    return result;
}
/*!
    Returns \a width for the texture.
*/
int Texture::width() const {
    return p_ptr->m_Width;
}
/*!
    Returns \a height for the texture.
*/
int Texture::height() const {
    return p_ptr->m_Height;
}
/*!
    Sets new \a width for the texture.
*/
void Texture::setWidth(int width) {
    p_ptr->m_Width = width;
}
/*!
    Sets new \a height for the texture.
*/
void Texture::setHeight(int height) {
    p_ptr->m_Height = height;
}
/*!
    \internal
*/
int32_t Texture::size(int32_t width, int32_t height) const {
    int32_t (Texture::*sizefunc)(int32_t, int32_t) const;
    sizefunc = (isCompressed() ? &Texture::sizeDXTc : &Texture::sizeRGB);

    return (this->*sizefunc)(width, height);
}
/*!
    \internal
*/
inline int32_t Texture::sizeDXTc(int32_t width, int32_t height) const {
    return ((width + 3) / 4) * ((height + 3) / 4) * (p_ptr->m_Compress == DXT1 ? 8 : 16);
}
/*!
    \internal
*/
inline int32_t Texture::sizeRGB(int32_t width, int32_t height) const {
    int32_t s = ((p_ptr->m_Format == RGB16Float ||
                  p_ptr->m_Format == RGBA32Float) ? 4 : 1);
    return width * height * components() * s;
}
/*!
    Returns the bounding shape for the texture.
*/
Vector2Vector Texture::shape() const {
    return p_ptr->m_Shape;
}
/*!
    Sets the bounding \a shape for the texture.
*/
void Texture::setShape(const Vector2Vector &shape) {
    p_ptr->m_Shape = shape;
}
/*!
    Sets new \a width and \a height for the texture.
*/
void Texture::resize(int width, int height) {
    clear();

    p_ptr->m_Width = width;
    p_ptr->m_Height = height;

    int32_t length = size(p_ptr->m_Width, p_ptr->m_Height);
    uint8_t *pixels = new uint8_t[length];
    memset(pixels, 0, length);
    Texture::Surface s;
    s.push_back(pixels);
    addSurface(s);

    setState(ToBeUpdated);
}
/*!
    Returns format type of texture.
    For more details please see the Texture::FormatType enum.
*/
Texture::FormatType Texture::format() const {
    return p_ptr->m_Format;
}
/*!
    Sets format \a type of texture.
    For more details please see the Texture::FormatType enum.
*/
void Texture::setFormat(FormatType type) {
    p_ptr->m_Format = type;
}
/*!
    \internal
*/
Texture::TextureType Texture::type() const {
    return p_ptr->m_Type;
}
/*!
    \internal
*/
void Texture::setType(TextureType type) {
    p_ptr->m_Type = type;
}
/*!
    Returns filtering type of texture.
    For more details please see the Texture::FilteringType enum.
*/
Texture::FilteringType Texture::filtering() const {
    return p_ptr->m_Filtering;
}
/*!
    Sets filtering \a type of texture.
    For more details please see the Texture::FilteringType enum.
*/
void Texture::setFiltering(FilteringType type) {
    p_ptr->m_Filtering = type;
}
/*!
    Returns the type of warp policy.
    For more details please see the Texture::WrapType enum.
*/
Texture::WrapType Texture::wrap() const {
    return p_ptr->m_Wrap;
}
/*!
    Sets the \a type of warp policy.
    For more details please see the Texture::WrapType enum.
*/
void Texture::setWrap(WrapType type) {
    p_ptr->m_Wrap = type;
}
/*!
    \internal
    Returns the number of texture sides.
    In most cases returns 1 but for the cube map will return 6
*/
Texture::Sides *Texture::getSides() {
    return &p_ptr->m_Sides;
}
/*!
    Returns true if texture uses one of the compression formats; otherwise returns false.
*/
bool Texture::isCompressed() const {
    return p_ptr->m_Compress != Uncompressed;
}
/*!
    Returns true if the texture is a cube map; otherwise returns false.
*/
bool Texture::isCubemap() const {
    return (p_ptr->m_Type == Cubemap);
}
/*!
    \internal
    Returns the number of the color channels(components)
*/
uint8_t Texture::components() const {
    switch(p_ptr->m_Format) {
        case R8: return 1;
        case RGB8:
        case RGB16Float:
        case R11G11B10Float: return 3;
        default: break;
    }
    return 4;
}
