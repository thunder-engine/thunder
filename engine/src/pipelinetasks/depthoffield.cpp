#include "pipelinetasks/depthoffield.h"

#include "engine.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"

#include "resources/material.h"
#include "resources/rendertarget.h"

namespace {
    const char *depthOfField("graphics.depthOfField");
};

DepthOfField::DepthOfField() :
        m_resultTexture(Engine::objectCreate<Texture>("depthOfField")),
        m_resultTarget(Engine::objectCreate<RenderTarget>()),
        m_dofMaterial(nullptr) {

    setName("DepthOfField");

    m_inputs.push_back("In");
    m_inputs.push_back("Downsample");
    m_inputs.push_back("Depth");

    Engine::setValue(depthOfField, 1);

    Material *material = Engine::loadResource<Material>(".embedded/DOF.shader");
    if(material) {
        m_dofMaterial = material->createInstance();
    }

    m_resultTexture->setFormat(Texture::R11G11B10Float);
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

void DepthOfField::exec(PipelineContext &context) {
    if(m_dofMaterial) {
        CommandBuffer *buffer = context.buffer();

        buffer->beginDebugMarker("DepthOfField");

        buffer->setRenderTarget(m_resultTarget);
        buffer->drawMesh(PipelineContext::defaultPlane(), 0, CommandBuffer::UI, *m_dofMaterial);

        buffer->endDebugMarker();
    }
}

void DepthOfField::setInput(int index, Texture *texture) {
    if(m_dofMaterial) {
        switch(index) {
            case 0: m_dofMaterial->setTexture("highMap", texture); break;
            case 1: m_dofMaterial->setTexture("lowMap", texture); break;
            case 2: m_dofMaterial->setTexture("depthMap", texture); break;
            default: break;
        }
    }
}
