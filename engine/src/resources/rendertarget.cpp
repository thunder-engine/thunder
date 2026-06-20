#include "resources/rendertarget.h"

/*!
    \class RenderTarget
    \brief Represents offscreen render pass.
    \inmodule Resources
*/

RenderTarget::RenderTarget() :
        m_depth(nullptr),
        m_flags(0),
        m_clearX(0),
        m_clearY(0),
        m_clearWidth(0),
        m_clearHeigh(0),
        m_currentTile(-1) {

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
    \fn int flags()

    Returns assigned flags for this render target.
*/
int RenderTarget::flags() const {
    return m_flags;
}
/*!
    \fn void setFlags(int flags)

    Sets a \a flags for this render target.
*/
void RenderTarget::setFlags(int flags) {
    m_flags = flags;
}
/*!
    Returns color that will be used to clear attached targets.
*/
const Vector4 &RenderTarget::clearColor() const {
    return m_clearColor;
}
/*!
    Sets the \a color that will be used to clear attached targets.
*/
void RenderTarget::setClearColor(const Vector4 &color) {
    m_clearColor = color;
}
/*!
    Retrieves the renderable area rectangle within this render target.

    This method returns the viewport or scissor rectangle that defines the
    actual rendering area available for drawing operations. The area is typically
    used for setting up the viewport, scissor test, or clearing operations.

    This method accepts several output parameters \a x \a y \a width and \a height.
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
    Returns current tile index.
*/
int32_t RenderTarget::tileIndex() const {
    return m_currentTile;
}
/*!
    \fn void setTileIndex(int32_t index)

    Sets a tile \a index required to setup global shader settings on some RHI's
*/
void RenderTarget::setTileIndex(int32_t index) {
    m_currentTile = index;
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
