#include "pipelinetasks/tonemap.h"

#include "components/private/postprocessorsettings.h"

#include "engine.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"

#include "resources/material.h"
#include "resources/rendertarget.h"

namespace {
    const char *lutMap("lutMap");
    const char *colorGradingLut("toneMap/colorGradingLut");
}

Tonemap::Tonemap() :
        m_resultTexture(Engine::objectCreate<Texture>("toneMap")),
        m_defaultLutTexture(Engine::objectCreate<Texture>("defaultLut")),
        m_lutTexture(nullptr),
        m_resultTarget(Engine::objectCreate<RenderTarget>()),
        m_resultMaterial(nullptr) {

    setName("Tonemap");

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

    m_inputs.push_back("In");

    PostProcessSettings::registerSetting(colorGradingLut, Variant::fromValue(m_lutTexture));

    Material *material = Engine::loadResource<Material>(".embedded/Tonemap.shader");
    if(material) {
        m_resultMaterial = material->createInstance();
        m_resultMaterial->setTexture(lutMap, m_defaultLutTexture);
    }

    m_resultTexture->setFormat(Texture::RGB8);
    m_resultTexture->setFlags(Texture::Render);

    m_resultTarget->setColorAttachment(0, m_resultTexture);

    m_outputs.push_back(std::make_pair(m_resultTexture->name(), m_resultTexture));
}

Tonemap::~Tonemap() {
    m_resultTarget->deleteLater();
}

void Tonemap::exec(PipelineContext &context) {
    if(m_resultMaterial) {
        CommandBuffer *buffer = context.buffer();

        for(auto it : context.culledPostEffectSettings()) {
            Texture *texture = it.first->readValue(colorGradingLut).value<Texture *>();
            if(texture != m_lutTexture) {
                m_lutTexture = texture;
                m_resultMaterial->setTexture(lutMap, m_lutTexture ? m_lutTexture : m_defaultLutTexture);
            }
        }

        buffer->beginDebugMarker("Tonemap");

        buffer->setRenderTarget(m_resultTarget);
        buffer->drawMesh(PipelineContext::defaultPlane(), 0, CommandBuffer::UI, *m_resultMaterial);

        buffer->endDebugMarker();
    }
}

void Tonemap::setInput(int index, Texture *texture) {
    if(m_resultMaterial) {
        m_resultMaterial->setTexture("rgbMap", texture);
    }
}
