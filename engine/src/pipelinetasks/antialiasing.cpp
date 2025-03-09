#include "pipelinetasks/antialiasing.h"

#include "engine.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"

#include "resources/material.h"
#include "resources/rendertarget.h"

namespace {
    const char *antialiasing("graphics.antialiasing");
};

AntiAliasing::AntiAliasing() :
        m_resultTexture(Engine::objectCreate<Texture>("antiAliasing")),
        m_resultTarget(Engine::objectCreate<RenderTarget>()),
        m_resultMaterial(nullptr) {

    m_enabled = false;
    setName("AntiAliasing");

    m_inputs.push_back("In");

    Engine::setValue(antialiasing, 1);

    Material *material = Engine::loadResource<Material>(".embedded/FXAA.shader");
    if(material) {
        m_resultMaterial = material->createInstance();
    }

    m_resultTexture->setFormat(Texture::RGBA16Float);
    m_resultTexture->setFlags(Texture::Render);

    m_resultTarget->setColorAttachment(0, m_resultTexture);

    m_outputs.push_back(std::make_pair(m_resultTexture->name(), m_resultTexture));
}

AntiAliasing::~AntiAliasing() {
    m_resultTarget->deleteLater();
}

void AntiAliasing::exec() {
    if(m_resultMaterial) {
        CommandBuffer *buffer = m_context->buffer();
        buffer->beginDebugMarker("AntiAliasing");

        buffer->setRenderTarget(m_resultTarget);
        buffer->drawMesh(PipelineContext::defaultPlane(), 0, CommandBuffer::UI, *m_resultMaterial);

        buffer->endDebugMarker();
    }
}

void AntiAliasing::setInput(int index, Texture *texture) {
    if(m_enabled) {
        if(m_resultMaterial) {
            m_resultMaterial->setTexture("rgbMap", texture);
        }
        m_outputs.back().second = m_resultTexture;
    } else {
        m_outputs.back().second = texture;
    }
}
