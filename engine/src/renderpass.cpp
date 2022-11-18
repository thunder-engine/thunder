#include "renderpass.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"

/*!
    \class RenderPass
    \brief A base class for all custom render passes.
    \inmodule Engine

    All render passes must be inherited from this class.
*/

RenderPass::RenderPass(PipelineContext *context) :
        m_enabled(true) {

}

RenderPass::~RenderPass() {

}
/*!
    The main method to apply post effect.
    The effect will be applied to \a source buffer for the provided \a pipeline.
*/
Texture *RenderPass::draw(Texture *source, PipelineContext *context) {
    return source;
}
/*!
    A callback to react on screen \a width and \a height changed.
*/
void RenderPass::resize(int32_t width, int32_t height) {

}
/*!
    A callback to react on chage of post effect \a settings.
*/
void RenderPass::setSettings(const PostProcessSettings &settings) {
    A_UNUSED(settings);
}
/*!
    Returns a layer where post effect will be called.
*/
uint32_t RenderPass::layer() const {
    return CommandBuffer::TRANSLUCENT;
}
/*!
    Returns a name of post effect.
*/
const char *RenderPass::name() const {
    return nullptr;
}
/*!
    Sets post effect to \a enable or disable.
    The disabled effect will not be applied.
*/
void RenderPass::setEnabled(bool enable) {
    m_enabled = enable;
}
/*!
    Returns true if post effect is enabled; otherwise returns false.
*/
bool RenderPass::isEnabled() const {
    return m_enabled;
}
