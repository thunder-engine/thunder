#include "resources/texture.h"

#include <variant.h>

#include <cstring>

namespace {
    const char *gData("Data");
}

uint32_t Texture::s_maxTextureSize = 1024;
uint32_t Texture::s_maxCubemapSize = 512;

/*!
    \class Texture
    \brief Texture resource contains all necessary texture data.
    \inmodule Resources

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

/*!
    \enum Texture::Flags

    \value Render \c This texture is used as render target in frame buffers.
    \value Feedback \c The feedback textures can read data from GPU to CPU.
*/

/*!
    \typedef Texture::Sides
    \internal
*/

/*!
    \typedef Texture::Surface
    \internal
*/

Texture::Texture() :
        m_format(Texture::R8),
        m_compress(Texture::Uncompressed),
        m_filtering(Texture::None),
        m_wrap(Texture::Clamp),
        m_width(1),
        m_height(1),
        m_depth(1),
        m_depthBits(0),
        m_flags(0) {

}

Texture::~Texture() {
    clear();
}
/*!
    \internal
*/
void Texture::loadUserData(const VariantMap &data) {
    clear();

    auto it = data.find(gData);
    if(it != data.end()) {
        const VariantList &surfaces = (*it).second.value<VariantList>();
        for(auto &s : surfaces) {
            Surface surface;

            for(auto &l : s.value<VariantList>()) {
                surface.push_back(l.toByteArray());
            }
            addSurface(surface);
        }
    }
}
/*!
    \internal
*/
VariantMap Texture::saveUserData() const {
    VariantMap result;

    VariantList surfaces;
    for(auto &side : m_sides) {
        VariantList surface;
        for(auto &lod : side) {
            surface.push_back(lod);
        }
        surfaces.push_back(surface);
    }
    result[gData] = surfaces;

    return result;
}
/*!
    Returns the number of texture sides.
    In most cases returns 1 but for the cube map will return 6
*/
int Texture::sides() const {
    return m_sides.size();
}
/*!
    Returns the number of MIP levels.
*/
int Texture::mipCount() const {
    return m_sides.empty() ? 1 : m_sides.front().size();
}
/*!
    Returns a surface for the provided \a side.
    Each texture must contain at least one surface.
    Commonly used to set surfaces for the cube maps.
*/
Texture::Surface &Texture::surface(int side) {
    return m_sides[side];
}
/*!
    Adds \a surface to the texture.
    Each texture must contain at least one surface.
*/
void Texture::addSurface(const Surface &surface) {
    m_sides.push_back(surface);
}
/*!
    Marks texture as dirty.
    That means this texture must be forcefully reloaded.
*/
void Texture::setDirty() {
    switchState(ToBeUpdated);
}
/*!
    Read pixels from GPU at \a x and \a y position with \a width and \a height dimensions into texture data.
*/
void Texture::readPixels(int x, int y, int width, int height) {
    A_UNUSED(x);
    A_UNUSED(y);
    A_UNUSED(width);
    A_UNUSED(height);
}
/*!
    Returns pixel color from mip \a level at \a x and \a y position as RGBA integer for example 0x00ff00ff which can be mapped to (0, 255, 0, 255)
*/
int Texture::getPixel(int x, int y, int level) const {
    uint32_t result = 0;
    if(!m_sides.empty() && m_sides[0].size() > level) {
        const uint8_t *ptr = m_sides[0][level].data() + (y * m_width + x) * 4;
        memcpy(&result, ptr, sizeof(uint32_t));
    }
    return result;
}
/*!
    Returns texture data from a mip \a level.
*/
ByteArray Texture::getPixels(int level) const {
    if(!m_sides.empty() && m_sides[0].size() > level) {
        return m_sides[0][level];
    }
    return ByteArray();
}
/*!
    Returns width for the texture.
*/
int Texture::width() const {
    return m_width;
}
/*!
    Sets new \a width for the texture.
*/
void Texture::setWidth(int width) {
    resize(width, m_height);
}
/*!
    Returns height for the texture.
*/
int Texture::height() const {
    return m_height;
}
/*!
    Sets new \a height for the texture.
*/
void Texture::setHeight(int height) {
    resize(m_width, height);
}
/*!
    Returns depth dimension for the texture.
*/
int Texture::depth() const {
    return m_depth;
}
/*!
    Sets new \a depth dimension for the texture.
*/
void Texture::setDepth(int depth) {
    m_depth = depth;
}
/*!
    Sets new \a width and \a height for the texture.
*/
void Texture::resize(int width, int height) {
    if((m_width != width || m_height != height) && width > 0 && height > 0) {
        m_width = width;
        m_height = height;

        if(!(m_flags & Flags::Render) || (m_flags & Flags::Feedback)) {
            clear();

            ByteArray pixels;
            pixels.resize(sizeRGB(m_width, m_height, m_depth));

            addSurface({pixels});
        }

        switchState(ToBeUpdated);
    }
}
/*!
    Returns format type of texture.
    For more details please see the Texture::FormatType enum.
*/
int Texture::format() const {
    return m_format;
}
/*!
    Sets format \a type of texture.
    For more details please see the Texture::FormatType enum.
*/
void Texture::setFormat(int type) {
    m_format = type;
}
/*!
    Returns filtering type of texture.
    For more details please see the Texture::FilteringType enum.
*/
int Texture::filtering() const {
    return m_filtering;
}
/*!
    Sets filtering \a type of texture.
    For more details please see the Texture::FilteringType enum.
*/
void Texture::setFiltering(int type) {
    m_filtering = type;
}
/*!
    Returns the type of warp policy.
    For more details please see the Texture::WrapType enum.
*/
int Texture::wrap() const {
    return m_wrap;
}
/*!
    Sets the \a type of warp policy.
    For more details please see the Texture::WrapType enum.
*/
void Texture::setWrap(int type) {
    m_wrap = type;
}
/*!
    Returns the number of depth buffer bits.
    \note This value is valid only for the depth textures.
*/
int Texture::depthBits() const {
    return m_depthBits;
}
/*!
    Sets the number of \a depth buffer bits.
    \note This value is valid only for the depth textures.
*/
void Texture::setDepthBits(int depth) {
    m_depthBits = depth;
}
/*!
    Returns true if texture is can be attached to framebuffer; otherwise returns false.
*/
bool Texture::isRender() const {
    return m_flags & Flags::Render;
}
/*!
    Returns true if texture marked as a feed back texture; otherwise returns false.
    The feedback textures can read data from GPU to CPU.
*/
bool Texture::isFeedback() const {
    return m_flags & Flags::Feedback;
}
/*!
    Returns service flags for the texture.

    \sa Texture::Flags
*/
int Texture::flags() const {
    return m_flags;
}
/*!
    Sets service \a flags for the texture.

    \sa Texture::Flags
*/
void Texture::setFlags(int flags) {
    m_flags = flags;

    if(isFeedback() && sides() == 0) {
        ByteArray pixels;
        pixels.resize(sizeRGB(m_width, m_height, m_depth));

        addSurface({pixels});
    }
}
/*!
    Returns compression method.
*/
int Texture::compress() const {
    return m_compress;
}
/*!
    Set the compression \a method.
*/
void Texture::setCompress(int method) {
    m_compress = method;
}
/*!
    Returns true if the texture is a cube map; otherwise returns false.
*/
bool Texture::isCubemap() const {
    return (m_sides.size() == 6);
}
/*!
    Returns true if texture provides a set of textures; otherwise returns false.
    \note For now will always return false.
*/
bool Texture::isArray() const {
    return false;
}
/*!
    \internal
    Returns the number of the color channels(components)
*/
uint8_t Texture::components() const {
    switch(m_format) {
        case R8: return 1;
        case RGB8:
        case R11G11B10Float: return 3;
        default: break;
    }
    return 4;
}
/*!
    \internal
*/
void Texture::switchState(State state) {
    setState(state);
}
/*!
    \internal
*/
bool Texture::isUnloadable() {
    return true;
}
/*!
    \internal
*/
void Texture::clear() {
    m_sides.clear();
}
/*!
    Returns the maximum texure size.
*/
uint32_t Texture::maxTextureSize() {
    return s_maxTextureSize;
}
/*!
    \internal
*/
void Texture::setMaxTextureSize(uint32_t size) {
    s_maxTextureSize = size;
}
/*!
    Returns the maximum cubemap size.
*/
uint32_t Texture::maxCubemapSize() {
    return s_maxCubemapSize;
}
/*!
    \internal
*/
void Texture::setMaxCubemapSize(uint32_t size) {
    s_maxCubemapSize = size;
}
/*!
    \internal
*/
inline int32_t Texture::sizeRGB(int32_t width, int32_t height, int32_t depth) const {
    int32_t s = 1;
    switch(m_format) {
        case RGBA32Float: s = 4; break;
        case RGBA16Float: s = 2; break;
        default: break;
    }
    return width * height * depth * components() * s;
}
/*!
    \internal
*/
bool Texture::isDwordAligned() {
    int dwordLineSize = dwordAlignedLineSize(width(), components() * 8);
    int curLineSize = width() * components();

    return (dwordLineSize == curLineSize);
}
/*!
    \internal
*/
inline int32_t Texture::dwordAlignedLineSize(int32_t width, int32_t bpp) {
    return ((width * bpp + 31) & -32) >> 3;
}
