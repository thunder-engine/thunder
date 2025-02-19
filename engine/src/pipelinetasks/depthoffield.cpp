#include "pipelinetasks/depthoffield.h"

#include "engine.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"

#include "resources/material.h"
#include "resources/rendertarget.h"

namespace {
    const char *gDepthOfField("graphics.depthOfField");

    const char *gFocusDistance("focusDistance");
    const char *gFocusScale("focusScale");
};

DepthOfField::DepthOfField() :
        m_resultTexture(Engine::objectCreate<Texture>("depthOfField")),
        m_resultTarget(Engine::objectCreate<RenderTarget>()),
        m_dofMaterial(nullptr),
        m_focusDistance(10.0f),
        m_focusScale(0.05f) {

    m_enabled = false;
    setName("DepthOfField");

    m_inputs.push_back("In");
    m_inputs.push_back("Downsample");
    m_inputs.push_back("Depth");

    Engine::setValue(gDepthOfField, false);

    Material *material = Engine::loadResource<Material>(".embedded/DOF.shader");
    if(material) {
        m_dofMaterial = material->createInstance();
        m_dofMaterial->setFloat(gFocusDistance, &m_focusDistance);
        m_dofMaterial->setFloat(gFocusScale, &m_focusScale);
    }

    m_resultTexture->setFormat(Texture::RGBA16Float);
    m_resultTexture->setFiltering(Texture::Bilinear);
    m_resultTexture->setFlags(Texture::Render);

    m_resultTarget->setColorAttachment(0, m_resultTexture);

    m_outputs.push_back(std::make_pair(m_resultTexture->name(), m_resultTexture));
}

DepthOfField::~DepthOfField() {
    m_resultTarget->deleteLater();
    m_resultTexture->deleteLater();

    delete m_dofMaterial;
}

void DepthOfField::exec() {
    if(m_dofMaterial) {
        CommandBuffer *buffer = m_context->buffer();

        buffer->beginDebugMarker("DepthOfField");

        buffer->setRenderTarget(m_resultTarget);
        buffer->drawMesh(PipelineContext::defaultPlane(), 0, CommandBuffer::UI, *m_dofMaterial);

        buffer->endDebugMarker();
    }
}

void DepthOfField::setInput(int index, Texture *texture) {
    if(m_enabled) {
        if(m_dofMaterial) {
            switch(index) {
                case 0: m_dofMaterial->setTexture("highMap", texture); break;
                case 1: m_dofMaterial->setTexture("lowMap", texture); break;
                case 2: m_dofMaterial->setTexture("depthMap", texture); break;
                default: break;
            }
        }
        m_resultTexture->resize(m_width, m_height);

        m_outputs.back().second = m_resultTexture;
    } else if(index == 0) {
        m_outputs.back().second = texture;
    }
}
