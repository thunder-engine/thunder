#include "postprocess/postprocessor.h"

#include "resources/mesh.h"
#include "resources/material.h"
#include "resources/rendertexture.h"

#include "commandbuffer.h"

#include "filters/blur.h"

static Blur *s_pBlur = nullptr;

PostProcessor::PostProcessor() :
        m_pResultTexture(nullptr),
        m_pMaterial(nullptr) {

    m_pMesh = Engine::loadResource<Mesh>(".embedded/plane.fbx/Plane001");
}

PostProcessor::~PostProcessor() {

}

RenderTexture *PostProcessor::draw(RenderTexture *source, ICommandBuffer &buffer) {
    if(m_pMaterial && m_pResultTexture) {
        m_pMaterial->setTexture("rgbMap", source);

        buffer.setRenderTarget({m_pResultTexture});
        buffer.drawMesh(Matrix4(), m_pMesh, ICommandBuffer::UI, m_pMaterial);

        return m_pResultTexture;
    }
    return source;
}

void PostProcessor::resize(int32_t width, int32_t height) {
    if(m_pResultTexture) {
        m_pResultTexture->resize(width, height);
    }
}

void PostProcessor::setSettings(const PostProcessSettings &) {

}

Blur *PostProcessor::blur() {
    if(s_pBlur == nullptr) {
        s_pBlur = new Blur();
    }
    return s_pBlur;
}
