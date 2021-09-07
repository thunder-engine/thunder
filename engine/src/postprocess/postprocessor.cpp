#include "postprocess/postprocessor.h"

#include "resources/mesh.h"
#include "resources/material.h"
#include "resources/rendertarget.h"
#include "resources/pipeline.h"

#include "commandbuffer.h"

#include "filters/blur.h"

static Blur *s_pBlur = nullptr;

/*!
    \class PostProcessor
    \brief A base class for all post effects.
    \inmodule Engine

    All post effects must be inherited from this class.
*/

PostProcessor::PostProcessor() :
        m_enabled(true),
        m_material(nullptr),
        m_resultTexture(nullptr) {

    m_mesh = Engine::loadResource<Mesh>(".embedded/plane.fbx/Plane001");

    m_resultTarget = Engine::objectCreate<RenderTarget>();
}

PostProcessor::~PostProcessor() {

}
/*!
    The main method to apply post effect.
    The effect will be applied to \a source buffer for the provided \a pipeline.
*/
Texture *PostProcessor::draw(Texture *source, Pipeline *pipeline) {
    if(m_enabled && m_material) {
        m_material->setTexture("rgbMap", source);

        CommandBuffer *buffer = pipeline->buffer();

        buffer->setRenderTarget(m_resultTarget);
        buffer->drawMesh(Matrix4(), m_mesh, 0, CommandBuffer::UI, m_material);

        return m_resultTexture;
    }

    return source;
}
/*!
    A callback to react on screen \a width and \a height changed.
*/
void PostProcessor::resize(int32_t width, int32_t height) {
    if(m_resultTexture) {
        m_resultTexture->setWidth(width);
        m_resultTexture->setHeight(height);
    }
}
/*!
    A callback to react on chage of post effect \a settings.
*/
void PostProcessor::setSettings(const PostProcessSettings &settings) {
    A_UNUSED(settings);
}
/*!
    Returns a layer where post effect will be called.
*/
uint32_t PostProcessor::layer() const {
    return CommandBuffer::TRANSLUCENT;
}
/*!
    Returns a name of post effect.
*/
const char *PostProcessor::name() const {
    return nullptr;
}
/*!
    Sets post effect to \a enable or disable.
    The disabled effect will not be applied.
*/
void PostProcessor::setEnabled(bool enable) {
    m_enabled = enable;
}
/*!
    Returns true if post effect is enabled; otherwise returns false.
*/
bool PostProcessor::isEnabled() const {
    return m_enabled;
}
/*!
    \internal
*/
Blur *PostProcessor::blur() {
    if(s_pBlur == nullptr) {
        s_pBlur = new Blur();
    }
    return s_pBlur;
}
