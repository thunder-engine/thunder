#include "pipelinetasks/reflections.h"

#include "resources/material.h"
#include "resources/rendertarget.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"

#include "amath.h"

namespace {
    const char *gReflections("graphics.reflections");
};

Reflections::Reflections() :
        m_sslrMaterial(nullptr),
        m_sslrTexture(Engine::objectCreate<Texture>("localReflections")),
        m_sslrTarget(Engine::objectCreate<RenderTarget>("localReflections")) {

    setName("Reflections");

    Engine::setValue(gReflections, true);

    m_inputs.push_back("LastFrame");
    m_inputs.push_back("Normals");
    m_inputs.push_back("Params");
    m_inputs.push_back("Depth");

    m_outputs.push_back(std::make_pair(m_sslrTexture->name(), m_sslrTexture));

    m_sslrTexture->setFormat(Texture::RGB10A2);
    m_sslrTexture->setFlags(Texture::Render);

    m_sslrTarget->setColorAttachment(0, m_sslrTexture);

    Material *material = Engine::loadResource<Material>(".embedded/SSLR.shader");
    if(material) {
        m_sslrMaterial = material->createInstance();
    }
}

void Reflections::exec() {
    if(m_sslrMaterial) {
        CommandBuffer *buffer = m_context->buffer();

        buffer->beginDebugMarker("ScreenReflections");

        buffer->setRenderTarget(m_sslrTarget);
        buffer->drawMesh(PipelineContext::defaultPlane(), 0, CommandBuffer::UI, *m_sslrMaterial);

        buffer->endDebugMarker();
    }
}

void Reflections::setInput(int index, Texture *texture) {
    if(m_sslrMaterial) {
        switch(index) {
            case 0: { // lastFrame
                m_sslrMaterial->setTexture("emissiveMap", texture);
            } break;
            case 1: { // normalsMap
                m_sslrMaterial->setTexture("normalsMap", texture);
            } break;
            case 2: { // paramsMap
                m_sslrMaterial->setTexture("paramsMap", texture);
            } break;
            case 3: { // depthMap
                m_sslrMaterial->setTexture("depthMap", texture);
            } break;
            default: break;
        }
    }
}
