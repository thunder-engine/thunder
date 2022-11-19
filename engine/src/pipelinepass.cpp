#include "pipelinepass.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"

/*!
    \class PipelinePass
    \brief A base class for all custom render passes.
    \inmodule Engine

    All render passes must be inherited from this class.
*/

PipelinePass::PipelinePass() :
        m_enabled(true) {

}

PipelinePass::~PipelinePass() {

}
/*!
    The main method to apply post effect.
    The effect will be applied to \a source buffer for the provided \a pipeline.
*/
Texture *PipelinePass::draw(Texture *source, PipelineContext *context) {
    return source;
}
/*!
    A callback to react on screen \a width and \a height changed.
*/
void PipelinePass::resize(int32_t width, int32_t height) {

}
/*!
    Set a \a source texture to use it in the render pass.
*/
void PipelinePass::setInput(uint32_t index, Texture *source) {
    m_inputs[index] = source;
}
/*!
    Returns a result of pass.
*/
Texture *PipelinePass::output() {
    return nullptr;
}
/*!
    A callback to react on chage of post effect \a settings.
*/
void PipelinePass::setSettings(const PostProcessSettings &settings) {
    A_UNUSED(settings);
}
/*!
    Returns a layer where post effect will be called.
*/
uint32_t PipelinePass::layer() const {
    return CommandBuffer::TRANSLUCENT;
}
/*!
    Sets post effect to \a enable or disable.
    The disabled effect will not be applied.
*/
void PipelinePass::setEnabled(bool enable) {
    m_enabled = enable;
}
/*!
    Returns true if post effect is enabled; otherwise returns false.
*/
bool PipelinePass::isEnabled() const {
    return m_enabled;
}
