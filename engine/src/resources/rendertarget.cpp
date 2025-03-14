#include "resources/rendertarget.h"

/*!
    \class RenderTarget
    \brief Represents offscreen render pass.
    \inmodule Resources
*/

RenderTarget::RenderTarget() :
        m_depth(nullptr),
        m_clearFlags(DoNothing),
        m_clearX(0),
        m_clearY(0),
        m_clearWidth(0),
        m_clearHeigh(0) {

}

RenderTarget::~RenderTarget() {

}
/*!
    Returns the number of attached color textures.
*/
uint32_t RenderTarget::colorAttachmentCount() const {
    return m_color.size();
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
    Returns clear buffers startegy used on render target bind.
*/
int RenderTarget::clearFlags() const {
    return m_clearFlags;
}
/*!
    Sets clear buffers startegy on bind using clear \a flags.
*/
void RenderTarget::setClearFlags(int flags) {
    m_clearFlags = flags;
}
/*!
    Returns an area available for rendering.
*/
void RenderTarget::renderArea(int32_t &x, int32_t &y, int32_t &width, int32_t &height) const {
    x = m_clearX;
    y = m_clearY;
    width = m_clearWidth;
    height = m_clearHeigh;
}
/*!
    Sets rendering area at \a x \a y position and \a width \a height dimensions.
*/
void RenderTarget::setRenderArea(int32_t x, int32_t y, int32_t width, int32_t height) {
    m_clearX = x;
    m_clearY = y;
    m_clearWidth = width;
    m_clearHeigh = height;
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
