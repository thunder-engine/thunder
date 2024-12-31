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
        m_resultTarget(Engine::objectCreate<RenderTarget>()),
        m_material(nullptr) {

    setName("AntiAliasing");

    m_inputs.push_back("In");

    Engine::setValue(antialiasing, 1);

    Material *material = Engine::loadResource<Material>(".embedded/FXAA.shader");
    if(material) {
        m_material = material->createInstance();
    }

    Texture *resultTexture = Engine::objectCreate<Texture>("AntiAliasing");
    resultTexture->setFormat(Texture::RGB10A2);
    resultTexture->setFlags(Texture::Render);
    m_outputs.push_back(std::make_pair("Result", resultTexture));

    m_resultTarget->setColorAttachment(0, resultTexture);

}

void AntiAliasing::exec(PipelineContext &context) {
    if(m_material) {
        CommandBuffer *buffer = context.buffer();
        buffer->beginDebugMarker("AntiAliasing");

        buffer->setRenderTarget(m_resultTarget);
        buffer->drawMesh(PipelineContext::defaultPlane(), 0, CommandBuffer::UI, *m_material);

        buffer->endDebugMarker();
    }
}

void AntiAliasing::setInput(int index, Texture *texture) {
    if(m_material) {
        m_material->setTexture("rgbMap", texture);
    }
}
