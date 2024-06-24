#include "resources/rendertarget.h"

/*!
    \class RenderTarget
    \brief Represents offscreen render pass.
    \inmodule Resources
*/

RenderTarget::RenderTarget() :
        m_depth(nullptr),
        m_native(false) {

}

RenderTarget::~RenderTarget() {

}
/*!
    Returns the number of attached color textures.
*/
uint32_t RenderTarget::colorAttachmentCount() const {
    size_t result = m_color.size();
    if(result == 0 && m_native) {
        ++result;
    }
    return result;
}
/*!
    Returns the attached color textures with \a index.
*/
Texture *RenderTarget::colorAttachment(uint32_t index) const {
    if(index < m_color.size()) {
        return m_color[index];
    }
    return nullptr;
}
/*!
    Attach a color \a texture at \a index to render target.
*/
uint32_t RenderTarget::setColorAttachment(uint32_t index, Texture *texture) {
    if(index < m_color.size()) {
        m_color[index] = texture;
        return index;
    } else {
        m_color.push_back(texture);
        return m_color.size() - 1;
    }
}
/*!
    Returns an attached depth texture if exist.
*/
Texture *RenderTarget::depthAttachment() const {
    return m_depth;
}
/*!
    Attach a depth \a texture to render target.
*/
void RenderTarget::setDepthAttachment(Texture *texture) {
    m_depth = texture;
}
/*!
    \internal
*/
void RenderTarget::makeNative() {
    m_native = true;
}
/*!
    \internal
*/
bool RenderTarget::isNative() const {
    return m_native;
}
/*!
    \internal
*/
void RenderTarget::switchState(State state) {
    setState(state);
}
/*!
    \internal
*/
bool RenderTarget::isUnloadable() {
    return true;
}
