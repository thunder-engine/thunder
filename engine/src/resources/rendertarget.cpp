#include "resources/rendertarget.h"

class RenderTargetPrivate {
public:
    RenderTargetPrivate() :
        m_depth(nullptr) {

    }

    vector<Texture *> m_color;

    Texture *m_depth;

    bool m_native = false;
};

/*!
    \class RenderTarget
    \brief Represents offscreen render pass.
    \inmodule Resources
*/

RenderTarget::RenderTarget() :
        p_ptr(new RenderTargetPrivate) {

}

RenderTarget::~RenderTarget() {
    delete p_ptr;
}
/*!
    Returns the number of attached color textures.
*/
uint32_t RenderTarget::colorAttachmentCount() const {
    return p_ptr->m_color.size();
}
/*!
    Returns the attached color textures with \a index.
*/
Texture *RenderTarget::colorAttachment(uint32_t index) const {
    if(index < p_ptr->m_color.size()) {
        return p_ptr->m_color[index];
    }
    return nullptr;
}
/*!
    Attach a color \a texture at \a index to render target.
*/
uint32_t RenderTarget::setColorAttachment(uint32_t index, Texture *texture) {
    if(index < p_ptr->m_color.size()) {
        p_ptr->m_color[index] = texture;
        return index;
    } else {
        p_ptr->m_color.push_back(texture);
        return p_ptr->m_color.size() - 1;
    }
}
/*!
    Returns an attached depth texture if exist.
*/
Texture *RenderTarget::depthAttachment() const {
    return p_ptr->m_depth;
}
/*!
    Attach a depth \a texture to render target.
*/
void RenderTarget::setDepthAttachment(Texture *texture) {
    p_ptr->m_depth = texture;
}
/*!
    \internal
*/
void RenderTarget::makeNative() {
    p_ptr->m_native = true;
}
/*!
    \internal
*/
void RenderTarget::switchState(ResourceState state) {
    setState(state);
}
