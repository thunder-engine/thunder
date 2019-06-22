#include "resources/rendertexture.h"

RenderTexture::RenderTexture() :
        m_DepthBits(0),
        m_Fixed(false) {

}

void RenderTexture::setTarget(FormatType format) {
    setFormat(format);
}

void RenderTexture::setDepth(uint8_t bits) {
    m_DepthBits = bits;
}

void RenderTexture::setFixed(bool fixed) {
    m_Fixed = fixed;
}

void RenderTexture::resize(int32_t width, int32_t height) {
    if(!m_Fixed) {
        setWidth(width);
        setHeight(height);
        apply();
    }
}

void RenderTexture::makeCurrent(uint32_t index) const {
    A_UNUSED(index)
}
