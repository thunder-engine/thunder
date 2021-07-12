#include "postprocess/postprocessor.h"

#include "resources/mesh.h"
#include "resources/material.h"
#include "resources/rendertarget.h"
#include "resources/pipeline.h"

#include "commandbuffer.h"

#include "filters/blur.h"

static Blur *s_pBlur = nullptr;

PostProcessor::PostProcessor() :
        m_enabled(false),
        m_material(nullptr),
        m_resultTexture(nullptr) {

    m_mesh = Engine::loadResource<Mesh>(".embedded/plane.fbx/Plane001");

    m_resultTarget = Engine::objectCreate<RenderTarget>();
}

PostProcessor::~PostProcessor() {

}

Texture *PostProcessor::draw(Texture *source, Pipeline *pipeline) {
    if(m_enabled && m_material) {
        m_material->setTexture("rgbMap", source);

        ICommandBuffer *buffer = pipeline->buffer();

        buffer->setRenderTarget(m_resultTarget);
        buffer->drawMesh(Matrix4(), m_mesh, ICommandBuffer::UI, m_material);

        return m_resultTexture;
    }

    return source;
}

void PostProcessor::resize(int32_t width, int32_t height) {
    if(m_resultTexture) {
        m_resultTexture->setWidth(width);
        m_resultTexture->setHeight(height);
    }
}

void PostProcessor::setSettings(const PostProcessSettings &) {

}

uint32_t PostProcessor::layer() const {
    return ICommandBuffer::TRANSLUCENT;
}

void PostProcessor::setEnabled(bool value) {
    m_enabled = value;
}

Blur *PostProcessor::blur() {
    if(s_pBlur == nullptr) {
        s_pBlur = new Blur();
    }
    return s_pBlur;
}
