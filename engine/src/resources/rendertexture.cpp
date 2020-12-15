#include "resources/rendertexture.h"

class RenderTexturePrivate {
public:
    RenderTexturePrivate() :
            m_DepthBits(0),
            m_Fixed(false) {

    }

    int m_DepthBits;

    bool m_Fixed;
};

/*!
    \class RenderTexture
    \brief Render textures are textures that can be rendered to.
    \inmodule Resource
*/

RenderTexture::RenderTexture() :
        p_ptr(new RenderTexturePrivate) {

}

RenderTexture::~RenderTexture() {
    delete p_ptr;
}
/*!
    Sets the new \a format type for the texture and sets resource state to ResourceState::ToBeUpdated.
    For more details please see the Texture::FormatType enum.
*/
void RenderTexture::setTarget(FormatType format) {
    setFormat(format);
    setState(ToBeUpdated);
}
/*!
    Returns the precision of the render texture's depth buffer in bits.
*/
int RenderTexture::depth() const {
    return p_ptr->m_DepthBits;
}
/*!
    Sets the precision of the render texture's depth buffer in \a bits.
*/
void RenderTexture::setDepth(int bits) {
    p_ptr->m_DepthBits = bits;
    setState(ToBeUpdated);
}
/*!
    Sets the \a fixed flag for the render texture. If true the resize() method will not take effect.
*/
void RenderTexture::setFixed(bool fixed) {
    p_ptr->m_Fixed = fixed;
}
/*!
    Changes current size of the render texture with new \a width, \a height and sets resource state to ResourceState::ToBeUpdated.
*/
void RenderTexture::resize(int width, int height) {
    if(!p_ptr->m_Fixed && (Texture::width() != width || Texture::height() != height)) {
        setWidth(width);
        setHeight(height);

        setState(ToBeUpdated);
    }
}
/*!
    \internal
*/
void RenderTexture::makeCurrent(uint32_t index) const {
    A_UNUSED(index);
}
