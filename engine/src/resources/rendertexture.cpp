#include "resources/rendertexture.h"

RenderTexture::RenderTexture() :
        m_DepthBits(0),
        m_Fixed(false) {

}

void RenderTexture::setTarget(FormatType format) {
    m_Format    = format;
}

void RenderTexture::setDepth(uint8_t bits) {
    m_DepthBits = bits;
}

void RenderTexture::setFixed(bool fixed) {
    m_Fixed = fixed;
}

void RenderTexture::resize(uint32_t width, uint32_t height) {
    if(!m_Fixed) {
        m_Width     = width;
        m_Height    = height;
        apply();
    }
}
