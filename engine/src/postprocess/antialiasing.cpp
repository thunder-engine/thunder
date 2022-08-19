#include "postprocess/antialiasing.h"

#include "engine.h"

#include "pipelinecontext.h"
#include "material.h"

#include "resources/rendertarget.h"

namespace {
    const char *antialiasing("graphics.antialiasing");
};

#include "commandbuffer.h"

AntiAliasing::AntiAliasing() {
    Material *material = Engine::loadResource<Material>(".embedded/SSAA.shader");
    if(material) {
        m_material = material->createInstance();
    }

    m_resultTexture = Engine::objectCreate<Texture>();
    m_resultTexture->setFormat(Texture::R11G11B10Float);

    m_resultTarget->setColorAttachment(0, m_resultTexture);

    Engine::setValue(antialiasing, true);
}

uint32_t AntiAliasing::layer() const {
    return CommandBuffer::UI;
}

const char *AntiAliasing::name() const {
    return "AntiAliasing";
}

