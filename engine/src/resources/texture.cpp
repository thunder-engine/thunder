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

Texture::Texture() :
        p_ptr(new TexturePrivate) {

    setShape({ Vector2(0.0f), Vector2(0.0f, 1.0f), Vector2(1.0f), Vector2(1.0f, 0.0f) });
}

Texture::~Texture() {
    clear();

    delete p_ptr;
}

void Texture::loadUserData(const VariantMap &data) {
    clear();

    {
        auto it = data.find(HEADER);
        if(it != data.end()) {
            VariantList header  = (*it).second.value<VariantList>();

            auto i      = header.begin();
            p_ptr->m_Width  = (*i).toInt();
            i++;
            p_ptr->m_Height = (*i).toInt();
            i++;
            //Reserved
            i++;

            p_ptr->m_Type      = TextureType((*i).toInt());
            i++;
            p_ptr->m_Compress  = CompressionType((*i).toInt());
            i++;
            p_ptr->m_Format    = FormatType((*i).toInt());
            i++;
            p_ptr->m_Filtering = FilteringType((*i).toInt());
            i++;
            p_ptr->m_Wrap      = WrapType((*i).toInt());
            i++;
        }
    }
    {
        auto it = data.find(DATA);
        if(it != data.end()) {
            const VariantList &surfaces = (*it).second.value<VariantList>();
            for(auto s : surfaces) {
                Surface img;
                int32_t w  = p_ptr->m_Width;
                int32_t h  = p_ptr->m_Height;
                const VariantList &lods = s.value<VariantList>();
                for(auto l : lods) {
                    ByteArray bits = l.toByteArray();
                    uint32_t s  = size(w, h);
                    if(s && !bits.empty()) {
                        uint8_t *pixels = new uint8_t[s];
                        memcpy(pixels, &bits[0], s);
                        img.push_back(pixels);
                    }
                    w   = MAX(w / 2, 1);
                    h   = MAX(h / 2, 1);
                }
                p_ptr->m_Sides.push_back(img);
            }
        }
    }

    setState(ToBeUpdated);
}

Texture::Surface &Texture::surface(uint32_t face) {
    return p_ptr->m_Sides[face];
}

void Texture::addSurface(const Surface &surface) {
    p_ptr->m_Sides.push_back(surface);
}

void Texture::setDirty() {
    setState(ToBeUpdated);
}

void Texture::clear() {
    p_ptr->m_Width  = 1;
    p_ptr->m_Height = 1;

    for(auto side : p_ptr->m_Sides) {
        for(auto lod : side) {
            delete []lod;
        }
    }
    p_ptr->m_Sides.clear();
    p_ptr->m_Shape.clear();
}

void *Texture::nativeHandle() {
    return nullptr;
}

void Texture::readPixels(int32_t x, int32_t y, int32_t width, int32_t height) {
    A_UNUSED(x);
    A_UNUSED(y);
    A_UNUSED(width);
    A_UNUSED(height);
}

uint32_t Texture::getPixel(int32_t x, int32_t y) const {
    uint32_t result = 0;
    if(!p_ptr->m_Sides.empty() && !p_ptr->m_Sides[0].empty()) {
        uint8_t *ptr = p_ptr->m_Sides[0][0] + (y * p_ptr->m_Width + x);
        memcpy(&result, ptr, sizeof(uint32_t));
    }
    return result;
}

int32_t Texture::width() const {
    return p_ptr->m_Width;
}

int32_t Texture::height() const {
    return p_ptr->m_Height;
}

void Texture::setWidth(int32_t width) {
    p_ptr->m_Width = width;
}

void Texture::setHeight(int32_t height) {
    p_ptr->m_Height  = height;
}

int32_t Texture::size(int32_t width, int32_t height) const {
    int32_t (Texture::*sizefunc)(int32_t, int32_t) const;
    sizefunc = (isCompressed() ? &Texture::sizeDXTc : &Texture::sizeRGB);

    return (this->*sizefunc)(width, height);
}

inline int32_t Texture::sizeDXTc(int32_t width, int32_t height) const {
    return ((width + 3) / 4) * ((height + 3) / 4) * (p_ptr->m_Compress == DXT1 ? 8 : 16);
}

inline int32_t Texture::sizeRGB(int32_t width, int32_t height) const {
    return width * height * components() * ((p_ptr->m_Format == RGB16Float) ? 4 : 1);
}

Vector2Vector Texture::shape() const {
    return p_ptr->m_Shape;
}

void Texture::setShape(const Vector2Vector &shape) {
    p_ptr->m_Shape = shape;
}

void Texture::resize(int32_t width, int32_t height) {
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

Texture::FormatType Texture::format() const {
    return p_ptr->m_Format;
}

void Texture::setFormat(FormatType type) {
    p_ptr->m_Format = type;
}

Texture::TextureType Texture::type() const {
    return p_ptr->m_Type;
}

void Texture::setType(TextureType type) {
    p_ptr->m_Type = type;
}

Texture::FilteringType Texture::filtering() const {
    return p_ptr->m_Filtering;
}

void Texture::setFiltering(FilteringType type) {
    p_ptr->m_Filtering = type;
}

Texture::WrapType Texture::wrap() const {
    return p_ptr->m_Wrap;
}
void Texture::setWrap(WrapType type) {
    p_ptr->m_Wrap = type;
}

Texture::Sides *Texture::getSides() {
    return &p_ptr->m_Sides;
}

bool Texture::isCompressed() const {
    return p_ptr->m_Compress != Uncompressed;
}

bool Texture::isCubemap() const {
    return (p_ptr->m_Type == Cubemap);
}

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
