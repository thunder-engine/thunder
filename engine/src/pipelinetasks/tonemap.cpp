#include "pipelinetasks/tonemap.h"

#include "components/private/postprocessorsettings.h"

#include "engine.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"

#include "resources/material.h"
#include "resources/rendertarget.h"

namespace {
    const char *gIn("In");
    const char *gResult("Result");
    const char *gTonemap("Tonemap");
    const char *gLutMap("lutMap");
    const char *gColorGradingLut("toneMap/colorGradingLut");
}

Tonemap::Tonemap() :
        m_resultTexture(Engine::objectCreate<Texture>("toneMap")),
        m_defaultLutTexture(Engine::objectCreate<Texture>("defaultLut")),
        m_lutTexture(nullptr),
        m_resultTarget(Engine::objectCreate<RenderTarget>()),
        m_resultMaterial(nullptr) {

    setName(gTonemap);

    const int side = 16;
    const int step = 17;
    m_defaultLutTexture->setFormat(Texture::RGBA8);
    m_defaultLutTexture->setFiltering(Texture::Bilinear);
    m_defaultLutTexture->resize(side, side);
    m_defaultLutTexture->setDepth(side);

    ByteArray data;
    data.resize(side * side * side * 4);
    for(int d = 0; d < side; d++) {
        for(int h = 0; h < side; h++) {
            for(int w = 0; w < side; w++) {
                int row = d * side + h;
                int index = (row * side + w) * 4;
                data[index] = w * step;
                data[index + 1] = h * step;
                data[index + 2] = d * step;
                data[index + 3] = 255;
            }
        }
    }
    Texture::Surface &surface = m_defaultLutTexture->surface(0);
    surface.clear();
    surface.push_back(data);

    PostProcessSettings::registerSetting(gColorGradingLut, Variant::fromValue(m_lutTexture));

    Material *material = Engine::loadResource<Material>(".embedded/Tonemap.shader");
    if(material) {
        m_resultMaterial = material->createInstance();
        m_resultMaterial->setTexture(gLutMap, m_defaultLutTexture);
    }

    m_resultTexture->setFormat(Texture::RGBA8);
    m_resultTexture->setFlags(Texture::Render);

    m_resultTarget->setColorAttachment(0, m_resultTexture);

    m_inputs.push_back(gIn);
    m_outputs.push_back(std::make_pair(gResult, m_resultTexture));
}

Tonemap::~Tonemap() {
    m_resultTarget->deleteLater();
}

void Tonemap::exec() {
    if(m_resultMaterial) {
        CommandBuffer *buffer = m_context->buffer();

        for(auto it : m_context->culledPostEffectSettings()) {
            Texture *texture = it.first->readValue(gColorGradingLut).value<Texture *>();
            if(texture != m_lutTexture) {
                m_lutTexture = texture;
                m_resultMaterial->setTexture(gLutMap, m_lutTexture ? m_lutTexture : m_defaultLutTexture);
            }
        }

        buffer->beginDebugMarker(gTonemap);

        buffer->setRenderTarget(m_resultTarget);
        buffer->drawMesh(PipelineContext::defaultPlane(), 0, CommandBuffer::UI, *m_resultMaterial);

        buffer->endDebugMarker();
    }
}

void Tonemap::setInput(int index, Texture *texture) {
    if(m_enabled) {
        if(m_resultMaterial) {
            m_resultMaterial->setTexture("rgbMap", texture);
        }
        m_outputs.front().second = m_resultTexture;
    } else {
        m_outputs.front().second = texture;
    }
}
