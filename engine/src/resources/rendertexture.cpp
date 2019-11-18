#include "resources/rendertexture.h"

class RenderTexturePrivate {
public:
    RenderTexturePrivate() :
            m_DepthBits(0),
            m_Fixed(false) {

    }

    uint8_t m_DepthBits;

    bool m_Fixed;
};

RenderTexture::RenderTexture() :
        p_ptr(new RenderTexturePrivate) {

}

RenderTexture::~RenderTexture() {
    delete p_ptr;
}

void RenderTexture::setTarget(FormatType format) {
    setFormat(format);
    setState(ToBeUpdated);
}

uint8_t RenderTexture::depth () const {
    return p_ptr->m_DepthBits;
}

void RenderTexture::setDepth(uint8_t bits) {
    p_ptr->m_DepthBits = bits;
    setState(ToBeUpdated);
}

void RenderTexture::setFixed(bool fixed) {
    p_ptr->m_Fixed = fixed;
}

void RenderTexture::resize(int32_t width, int32_t height) {
    if(!p_ptr->m_Fixed && (Texture::width() != width || Texture::height() != height)) {
        setWidth(width);
        setHeight(height);

        setState(ToBeUpdated);
    }
}

void RenderTexture::makeCurrent(uint32_t index) const {
    A_UNUSED(index);
}
